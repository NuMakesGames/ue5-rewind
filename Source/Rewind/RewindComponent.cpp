// Fill out your copyright notice in the Description page of Project Settings.

#include "RewindComponent.h"

#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/MovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "RewindCharacter.h"
#include "RewindGameMode.h"

bool GVisualizeRewindSnapshots = false;
FAutoConsoleVariableRef CVarVisualizeRewindSnapshots(
	TEXT("Rewind.VisualizeSnapshots"),
	GVisualizeRewindSnapshots,
	TEXT("Whether to visualize stored snapshots"),
	ECVF_Cheat);

URewindComponent::URewindComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Tick after movement is completed
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void URewindComponent::BeginPlay()
{
	Super::BeginPlay();

	GameMode = Cast<ARewindGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		// Disable ticking if we can't find the rewind game mode; no hope of rewinding
		SetComponentTickEnabled(false);
		return;
	}

	// Grab owner's root component to manipulate physics during rewind
	OwnerRootComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());

	// If movement snapshotting is enabled, grab the owner's movement component
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (bSnapshotMovementVelocityAndMode)
	{
		OwnerMovementComponent = Character ? Cast<UCharacterMovementComponent>(Character->GetMovementComponent()) : nullptr;
	}

	// If configured to pause animations, grab the owner's skeletal mesh
	if (bPauseAnimationDuringTimeScrubbing) { OwnerSkeletalMesh = Character ? Character->GetMesh() : nullptr; }

	// Assign a random color for debug visualization
	DebugColor = FColor::MakeRandomColor();

	// Bind to rewind/time scrub start/complete events on the game mode
	GameMode->OnGlobalRewindStarted.AddUniqueDynamic(this, &URewindComponent::OnGlobalRewindStarted);
	GameMode->OnGlobalRewindCompleted.AddUniqueDynamic(this, &URewindComponent::OnGlobalRewindCompleted);
	GameMode->OnGlobalFastForwardStarted.AddUniqueDynamic(this, &URewindComponent::OnGlobalFastForwardStarted);
	GameMode->OnGlobalFastForwardCompleted.AddUniqueDynamic(this, &URewindComponent::OnGlobalFastForwardCompleted);
	GameMode->OnGlobalTimeScrubStarted.AddUniqueDynamic(this, &URewindComponent::OnGlobalTimeScrubStarted);
	GameMode->OnGlobalTimeScrubCompleted.AddUniqueDynamic(this, &URewindComponent::OnGlobalTimeScrubCompleted);

	// Preallocate the space required in the ring buffers
	InitializeRingBuffers(GameMode->MaxRewindSeconds);
}

void URewindComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsRewinding) { PlaySnapshots(DeltaTime, true /*bRewinding*/); }
	else if (bIsFastForwarding) { PlaySnapshots(DeltaTime, false /*bRewinding*/); }
	else if (bIsTimeScrubbing) { PauseTime(DeltaTime, bLastTimeManipulationWasRewind); }
	else { RecordSnapshot(DeltaTime); }

	DebugDrawSnapshots();
}

void URewindComponent::SetIsRewindingEnabled(bool bEnabled)
{
	bIsRewindingEnabled = bEnabled;
	if (!bIsRewindingEnabled)
	{
		// Stop rewinding if necessary
		if (bIsRewinding) { OnGlobalRewindCompleted(); }

		// Stop fast forwarding if necessary
		if (bIsFastForwarding) { OnGlobalFastForwardCompleted(); }

		// Stop time scrubbing if necessary
		if (bIsTimeScrubbing) { OnGlobalTimeScrubCompleted(); }
	}
	else
	{
		check(GameMode);

		// Start rewinding if a global rewind is in progress
		if (!bIsRewinding && GameMode->IsGlobalRewinding()) { OnGlobalRewindStarted(); }

		// Start fast forwarding if necessary
		if (!bIsFastForwarding && GameMode->IsGlobalFastForwarding()) { OnGlobalFastForwardStarted(); }

		// Start time scrubbing if a global time scrub is in progress
		if (!bIsTimeScrubbing && GameMode->IsGlobalTimeScrubbing()) { OnGlobalTimeScrubStarted(); }
	}
}

void URewindComponent::OnGlobalRewindStarted()
{
	// Attempt to start rewinding; reset TimeSinceSnapshostsChanged if not time scrubbing
	bool bAlreadyManipulatingTime = IsTimeBeingManipulated();
	if (TryStartTimeManipulation(bIsRewinding, !bIsTimeScrubbing))
	{
		// Notify event subscribers that rewind (and possibly time manipulation) started
		OnRewindStarted.Broadcast();
		if (!bAlreadyManipulatingTime) { OnTimeManipulationStarted.Broadcast(); }
	}
}

void URewindComponent::OnGlobalFastForwardStarted()
{
	// Fast forwarding is only allowed when time scrubbing
	// Attempt to start fast forwarding; reset TimeSinceSnapshostsChanged if not time scrubbing
	bool bAlreadyManipulatingTime = IsTimeBeingManipulated();
	if (bIsTimeScrubbing && TryStartTimeManipulation(bIsFastForwarding, !bIsTimeScrubbing))
	{
		// Notify event subscribers that fast forward (and possibly time manipulation) started
		OnFastForwardStarted.Broadcast();
		if (!bAlreadyManipulatingTime) { OnTimeManipulationStarted.Broadcast(); }
	}
}

void URewindComponent::OnGlobalTimeScrubStarted()
{
	// Attempt to start time scrubbing; never reset TimeSinceSnapshostsChanged for time scrubbing
	bool bAlreadyManipulatingTime = IsTimeBeingManipulated();
	if (TryStartTimeManipulation(bIsTimeScrubbing, false))
	{
		// Notify event subscribers that rewind (and possibly time manipulation) started
		OnTimeScrubStarted.Broadcast();
		if (!bAlreadyManipulatingTime) { OnTimeManipulationStarted.Broadcast(); }
	}
}

void URewindComponent::OnGlobalRewindCompleted()
{
	// Attempt to stop rewinding
	if (TryStopTimeManipulation(bIsRewinding, !bIsTimeScrubbing, false /*bResetMovementVelocity*/))
	{
		// Mark last operation as a rewind for subsequent time scrubbing
		bLastTimeManipulationWasRewind = true;

		// Notify event subscribers that rewind (and possibly time manipulation) completed
		OnRewindCompleted.Broadcast();
		if (!IsTimeBeingManipulated()) { OnTimeManipulationCompleted.Broadcast(); }
	}
}

void URewindComponent::OnGlobalFastForwardCompleted()
{
	// Attempt to stop fast forwarding
	if (TryStopTimeManipulation(bIsFastForwarding, !bIsTimeScrubbing, false /*bResetMovementVelocity*/))
	{
		// Mark last operation as a fast forward for subsequent time scrubbing
		bLastTimeManipulationWasRewind = false;

		// Notify event subscribers that fast forward (and possibly time manipulation) completed
		OnFastForwardCompleted.Broadcast();
		if (!IsTimeBeingManipulated()) { OnTimeManipulationCompleted.Broadcast(); }
	}
}

void URewindComponent::OnGlobalTimeScrubCompleted()
{
	// Attempt to stop time scrubbing; reset movement velocity to match player expectations
	if (TryStopTimeManipulation(bIsTimeScrubbing, false /*bResetTimeSinceSnapshotsChanged*/, true /*bResetMovementVelocity*/))
	{
		// Notify event subscribers that time scrubbing (and possibly time manipulation) completed
		OnFastForwardCompleted.Broadcast();
		if (!IsTimeBeingManipulated()) { OnTimeManipulationCompleted.Broadcast(); }
	}
}

void URewindComponent::InitializeRingBuffers(float MaxRewindSeconds)
{
	// Figure out how many snapshots we need to store
	MaxSnapshots = FMath::CeilToInt32(MaxRewindSeconds / SnapshotFrequencySeconds);

	// Make sure we're not accidentally allocating an obscene amount of memory
	constexpr uint32 OneMB = 1024 * 1024;
	constexpr uint32 ThreeMB = 3 * OneMB;
	if (!bSnapshotMovementVelocityAndMode)
	{
		uint32 SnapshotBytes = sizeof(FTransformAndVelocitySnapshot);
		uint32 TotalSnapshotBytes = MaxSnapshots * SnapshotBytes;
		ensureMsgf(
			TotalSnapshotBytes < OneMB,
			TEXT("Actor %s has rewind component that requested %d bytes of snapshots. Check snapshot frequency!"),
			*GetOwner()->GetName(),
			TotalSnapshotBytes);

		MaxSnapshots = FMath::Min(MaxSnapshots, static_cast<uint32>(OneMB / SnapshotBytes));
	}
	else
	{
		uint32 SnapshotBytes = sizeof(FTransformAndVelocitySnapshot) + sizeof(FMovementVelocityAndModeSnapshot);
		uint32 TotalSnapshotBytes = MaxSnapshots * SnapshotBytes;
		ensureMsgf(
			TotalSnapshotBytes < ThreeMB,
			TEXT("Actor %s has rewind component that requested %d bytes of snapshots. Check snapshot frequency!"),
			*GetOwner()->GetName(),
			TotalSnapshotBytes);

		MaxSnapshots = FMath::Min(MaxSnapshots, static_cast<uint32>(ThreeMB / SnapshotBytes));
	}

	// Initialize buffer
	TransformAndVelocitySnapshots.Reserve(MaxSnapshots);

	// Grab owner's root component to manipulate physics during rewind
	if (bSnapshotMovementVelocityAndMode && OwnerMovementComponent)
	{
		// Initialize buffer for movement component snapshots
		MovementVelocityAndModeSnapshots.Reserve(MaxSnapshots);
	}
}

void URewindComponent::RecordSnapshot(float DeltaTime)
{
	TimeSinceSnapshotsChanged += DeltaTime;

	// Early out if last snapshot was taken within the desired snapshot cadence
	if (TimeSinceSnapshotsChanged < SnapshotFrequencySeconds && TransformAndVelocitySnapshots.Num() != 0) { return; }

	// If the buffer is full, drop the oldest snapshot
	if (TransformAndVelocitySnapshots.Num() == MaxSnapshots) { TransformAndVelocitySnapshots.PopFront(); }

	// Record the transform and velocity
	FTransform Transform = GetOwner()->GetActorTransform();
	FVector LinearVelocity = OwnerRootComponent ? OwnerRootComponent->GetPhysicsLinearVelocity() : FVector::Zero();
	FVector AngularVelocityInRadians = OwnerRootComponent ? OwnerRootComponent->GetPhysicsAngularVelocityInRadians() : FVector::Zero();
	LatestSnapshotIndex =
		TransformAndVelocitySnapshots.Emplace(TimeSinceSnapshotsChanged, Transform, LinearVelocity, AngularVelocityInRadians);

	if (bSnapshotMovementVelocityAndMode && OwnerMovementComponent)
	{
		// If the buffer is full, drop the oldest snapshot
		if (MovementVelocityAndModeSnapshots.Num() == MaxSnapshots) { MovementVelocityAndModeSnapshots.PopFront(); }

		// Record the movement velocity and movement mode
		FVector MovementVelocity = OwnerMovementComponent->Velocity;
		TEnumAsByte<EMovementMode> MovementMode = OwnerMovementComponent->MovementMode;
		int32 LatestMovementSnapshotIndex =
			MovementVelocityAndModeSnapshots.Emplace(TimeSinceSnapshotsChanged, MovementVelocity, MovementMode);
		check(LatestSnapshotIndex == LatestMovementSnapshotIndex);
	}

	TimeSinceSnapshotsChanged = 0.0f;
}

void URewindComponent::EraseFutureSnapshots()
{
	// Pop snapshots until the latest snapshot is the last one in the buffer
	while (LatestSnapshotIndex < TransformAndVelocitySnapshots.Num() - 1)
	{
		TransformAndVelocitySnapshots.Pop();
	}

	if (bSnapshotMovementVelocityAndMode)
	{
		// Pop snapshots until the latest snapshot is the last one in the buffer
		while (LatestSnapshotIndex < MovementVelocityAndModeSnapshots.Num() - 1)
		{
			MovementVelocityAndModeSnapshots.Pop();
		}
	}
}

void URewindComponent::PlaySnapshots(float DeltaTime, bool bRewinding)
{
	UnpauseAnimation();

	if (HandleInsufficientSnapshots()) { return; }

	// Apply time dilation to delta time
	DeltaTime *= GameMode->GetGlobalRewindSpeed();
	TimeSinceSnapshotsChanged += DeltaTime;

	bool bReachedEndOfTrack = false;
	float LatestSnapshotTime = TransformAndVelocitySnapshots[LatestSnapshotIndex].TimeSinceLastSnapshot;
	if (bRewinding)
	{
		// Drop any snapshots that are too old to be relevant
		while (LatestSnapshotIndex > 0 && TimeSinceSnapshotsChanged > LatestSnapshotTime)
		{
			TimeSinceSnapshotsChanged -= LatestSnapshotTime;
			LatestSnapshotTime = TransformAndVelocitySnapshots[LatestSnapshotIndex].TimeSinceLastSnapshot;
			--LatestSnapshotIndex;
		}

		// If we don't have any snapshots in the future, we can't interpolate, so just snap to the latest snapshot
		if (LatestSnapshotIndex == TransformAndVelocitySnapshots.Num() - 1)
		{
			ApplySnapshot(TransformAndVelocitySnapshots[LatestSnapshotIndex], false /*bApplyPhysics*/);
			if (bSnapshotMovementVelocityAndMode)
			{
				ApplySnapshot(MovementVelocityAndModeSnapshots[LatestSnapshotIndex], true /*bApplyTimeDilationToVelocity*/);
			}
			return;
		}

		bReachedEndOfTrack = LatestSnapshotIndex == 0;
	}
	else
	{
		// Drop any snapshots that are too old to be relevant
		while (LatestSnapshotIndex < TransformAndVelocitySnapshots.Num() - 1 && TimeSinceSnapshotsChanged > LatestSnapshotTime)
		{
			TimeSinceSnapshotsChanged -= LatestSnapshotTime;
			LatestSnapshotTime = TransformAndVelocitySnapshots[LatestSnapshotIndex].TimeSinceLastSnapshot;
			++LatestSnapshotIndex;
		}

		bReachedEndOfTrack = LatestSnapshotIndex == TransformAndVelocitySnapshots.Num() - 1;
	}

	// If we've reached the end of our track, clamp the interpolation and repause animation
	if (bReachedEndOfTrack)
	{
		TimeSinceSnapshotsChanged = FMath::Min(TimeSinceSnapshotsChanged, LatestSnapshotTime);
		if (bAnimationsPausedAtStartOfTimeManipulation) { PauseAnimation(); }
	}

	InterpolateAndApplySnapshots(bRewinding);
}

void URewindComponent::PauseTime(float DeltaTime, bool bRewinding)
{
	if (HandleInsufficientSnapshots()) { return; }

	if (bRewinding)
	{
		// If we don't have any snapshots in the future, use the latest snapshot
		if (LatestSnapshotIndex == TransformAndVelocitySnapshots.Num() - 1)
		{
			ApplySnapshot(TransformAndVelocitySnapshots[LatestSnapshotIndex], false /*bApplyPhysics*/);
			if (bSnapshotMovementVelocityAndMode)
			{
				ApplySnapshot(MovementVelocityAndModeSnapshots[LatestSnapshotIndex], true /*bApplyTimeDilationToVelocity*/);
			}
			PauseAnimation();
			return;
		}
	}

	// Continue interpolation until we reach the next snapshot
	float LatestSnapshotTime = TransformAndVelocitySnapshots[LatestSnapshotIndex].TimeSinceLastSnapshot;
	if (TimeSinceSnapshotsChanged < LatestSnapshotTime)
	{
		// Apply time dilation clamped to snapshot time
		DeltaTime *= GameMode->GetGlobalRewindSpeed();
		TimeSinceSnapshotsChanged = FMath::Min(TimeSinceSnapshotsChanged + DeltaTime, LatestSnapshotTime);
	}

	InterpolateAndApplySnapshots(bRewinding);

	if (FMath::IsNearlyEqual(TimeSinceSnapshotsChanged, LatestSnapshotTime)) { PauseAnimation(); }
}

bool URewindComponent::TryStartTimeManipulation(bool& bStateToSet, bool bResetTimeSinceSnapshotsChanged)
{
	if (!bIsRewindingEnabled || bStateToSet) { return false; }

	// Turn on requested time manipulation (i.e. bIsRewinding, bIsFastForwarding, bIsTimeScrubbing)
	bStateToSet = true;

	// Caller may want to maintain current interpolation (ex. during time scrubbing)
	if (bResetTimeSinceSnapshotsChanged) { TimeSinceSnapshotsChanged = 0.0f; }

	// Physics simulation disabled during rewinding
	PausePhysics();

	// Check whether animations were paused when we started this time manipulation operation
	bAnimationsPausedAtStartOfTimeManipulation = bPausedAnimation;

	return true;
}

bool URewindComponent::TryStopTimeManipulation(bool& bStateToSet, bool bResetTimeSinceSnapshotsChanged, bool bResetMovementVelocity)
{
	if (!bStateToSet) { return false; }

	// Turn off requested time manipulation (i.e. bIsRewinding, bIsFastForwarding, bIsTimeScrubbing)
	bStateToSet = false;

	// If transitioning from rewind to regular play, restore state
	if (!bIsTimeScrubbing)
	{
		if (bResetTimeSinceSnapshotsChanged) { TimeSinceSnapshotsChanged = 0.0f; }

		// Resume physics simulation
		UnpausePhysics();

		// Restore animation
		UnpauseAnimation();

		// Snap to the last snapshot before exiting rewind
		if (LatestSnapshotIndex >= 0)
		{
			ApplySnapshot(TransformAndVelocitySnapshots[LatestSnapshotIndex], true /*bApplyPhysics*/);
			if (bSnapshotMovementVelocityAndMode)
			{
				ApplySnapshot(MovementVelocityAndModeSnapshots[LatestSnapshotIndex], false /*bApplyTimeDilationToVelocity*/);

				// Players will be surprised if they continue moving after time scrubbing; clear movement velocity
				if (bResetMovementVelocity && OwnerMovementComponent) { OwnerMovementComponent->Velocity = FVector::ZeroVector; }
			}
		}

		// Delete any future snapshots on the timeline that should be overwritten by new snapshots
		EraseFutureSnapshots();
	}

	return true;
}

void URewindComponent::PausePhysics()
{
	// Pause physics if it's simulating
	if (OwnerRootComponent && OwnerRootComponent->BodyInstance.bSimulatePhysics)
	{
		bPausedPhysics = true;
		OwnerRootComponent->SetSimulatePhysics(false);
	}
}

void URewindComponent::UnpausePhysics()
{
	// Restart physics if simulation was paused
	if (!bPausedPhysics) { return; }

	check(OwnerRootComponent);
	bPausedPhysics = false;
	OwnerRootComponent->SetSimulatePhysics(true);
	OwnerRootComponent->RecreatePhysicsState();
}

void URewindComponent::PauseAnimation()
{
	if (!bPauseAnimationDuringTimeScrubbing) { return; }

	check(OwnerSkeletalMesh);
	bPausedAnimation = true;
	OwnerSkeletalMesh->bPauseAnims = true;
}

void URewindComponent::UnpauseAnimation()
{
	if (!bPausedAnimation) { return; }

	check(OwnerSkeletalMesh);
	bPausedAnimation = false;
	OwnerSkeletalMesh->bPauseAnims = false;
}

bool URewindComponent::HandleInsufficientSnapshots()
{
	// Nothing to do if no snapshots are available
	check(!bSnapshotMovementVelocityAndMode || TransformAndVelocitySnapshots.Num() == MovementVelocityAndModeSnapshots.Num());
	if (LatestSnapshotIndex < 0 || TransformAndVelocitySnapshots.Num() == 0) { return true; }

	// If only one snapshot is available, snap to it
	if (TransformAndVelocitySnapshots.Num() == 1)
	{
		ApplySnapshot(TransformAndVelocitySnapshots[0], false /*bApplyPhysics*/);
		if (bSnapshotMovementVelocityAndMode) { ApplySnapshot(MovementVelocityAndModeSnapshots[0], true /*bApplyTimeDilationToVelocity*/); }
		return true;
	}

	// Sanity check invariant
	check(LatestSnapshotIndex >= 0 && LatestSnapshotIndex < TransformAndVelocitySnapshots.Num());
	return false;
}

void URewindComponent::InterpolateAndApplySnapshots(bool bRewinding)
{
	// Interpolate between the two relevant snapshots
	constexpr int MinSnapshotsForInterpolation = 2;
	check(TransformAndVelocitySnapshots.Num() >= MinSnapshotsForInterpolation);
	check(bRewinding && LatestSnapshotIndex < TransformAndVelocitySnapshots.Num() - 1 || !bRewinding && LatestSnapshotIndex > 0);
	int PreviousIndex = bRewinding ? LatestSnapshotIndex + 1 : LatestSnapshotIndex - 1;

	// Blend and apply transform and velocity snapshots (scoped to avoid variable shadowing)
	{
		const FTransformAndVelocitySnapshot& PreviousSnapshot = TransformAndVelocitySnapshots[PreviousIndex];
		const FTransformAndVelocitySnapshot& NextSnapshot = TransformAndVelocitySnapshots[LatestSnapshotIndex];
		ApplySnapshot(
			BlendSnapshots(PreviousSnapshot, NextSnapshot, TimeSinceSnapshotsChanged / NextSnapshot.TimeSinceLastSnapshot),
			false /*bApplyPhysics*/);
	}

	// Blend and apply movement velocity and mode snapshots
	if (bSnapshotMovementVelocityAndMode)
	{
		const FMovementVelocityAndModeSnapshot& PreviousSnapshot = MovementVelocityAndModeSnapshots[PreviousIndex];
		const FMovementVelocityAndModeSnapshot& NextSnapshot = MovementVelocityAndModeSnapshots[LatestSnapshotIndex];
		ApplySnapshot(
			BlendSnapshots(PreviousSnapshot, NextSnapshot, TimeSinceSnapshotsChanged / NextSnapshot.TimeSinceLastSnapshot),
			true /*bApplyTimeDilationToVelocity*/);
	}
}

URewindComponent::FTransformAndVelocitySnapshot URewindComponent::BlendSnapshots(
	const FTransformAndVelocitySnapshot& A,
	const FTransformAndVelocitySnapshot& B,
	float Alpha)
{
	Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
	FTransformAndVelocitySnapshot BlendedSnapshot;
	BlendedSnapshot.Transform.Blend(A.Transform, B.Transform, Alpha);
	BlendedSnapshot.LinearVelocity = FMath::Lerp(A.LinearVelocity, B.LinearVelocity, Alpha);
	BlendedSnapshot.AngularVelocityInRadians = FMath::Lerp(A.AngularVelocityInRadians, B.AngularVelocityInRadians, Alpha);
	return BlendedSnapshot;
}

URewindComponent::FMovementVelocityAndModeSnapshot URewindComponent::BlendSnapshots(
	const FMovementVelocityAndModeSnapshot& A,
	const FMovementVelocityAndModeSnapshot& B,
	float Alpha)
{
	Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
	FMovementVelocityAndModeSnapshot BlendedSnapshot;
	BlendedSnapshot.MovementVelocity = FMath::Lerp(A.MovementVelocity, B.MovementVelocity, Alpha);
	BlendedSnapshot.MovementMode = Alpha < 0.5f ? A.MovementMode : B.MovementMode;
	return BlendedSnapshot;
}

void URewindComponent::ApplySnapshot(const FTransformAndVelocitySnapshot& Snapshot, bool bApplyPhysics)
{
	GetOwner()->SetActorTransform(Snapshot.Transform);
	if (OwnerRootComponent && bApplyPhysics)
	{
		OwnerRootComponent->SetPhysicsLinearVelocity(Snapshot.LinearVelocity);
		OwnerRootComponent->SetPhysicsAngularVelocityInRadians(Snapshot.AngularVelocityInRadians);
	}
}

void URewindComponent::ApplySnapshot(const FMovementVelocityAndModeSnapshot& Snapshot, bool bApplyTimeDilationToVelocity)
{
	if (OwnerMovementComponent)
	{
		OwnerMovementComponent->Velocity =
			bApplyTimeDilationToVelocity ? Snapshot.MovementVelocity * GameMode->GetGlobalRewindSpeed() : Snapshot.MovementVelocity;
		OwnerMovementComponent->SetMovementMode(Snapshot.MovementMode);
	}
}

/*static*/ void URewindComponent::ToggleTimelineSplinesVisibility()
{
	GVisualizeRewindSnapshots = !GVisualizeRewindSnapshots;
}

void URewindComponent::DebugDrawSnapshots()
{
	if (!GVisualizeRewindSnapshots) { return; }

	for (int i = 0; i < TransformAndVelocitySnapshots.Num(); ++i)
	{
		const FTransformAndVelocitySnapshot& Snapshot = TransformAndVelocitySnapshots[i];
		FVector Location = Snapshot.Transform.GetLocation();
		DrawDebugPoint(GetWorld(), Location, i > LatestSnapshotIndex ? 8.0f : 10.0f, DebugColor);

		if (i != 0)
		{
			const FTransformAndVelocitySnapshot& PreviousSnapshot = TransformAndVelocitySnapshots[i - 1];
			FVector PreviousLocation = PreviousSnapshot.Transform.GetLocation();
			DrawDebugLine(
				GetWorld(), PreviousLocation, Location, i > LatestSnapshotIndex ? FColor::White : DebugColor, false, -1.0f, 0, 2.0f);
		}
	}
}
