// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"
#include "Runtime/Core/Public/Containers/RingBuffer.h"

#include "RewindComponent.generated.h"

class UCharacterMovementComponent;
class USkeletalMeshComponent;
class ARewindGameMode;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimeManipulationStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimeManipulationCompleted);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRewindStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRewindCompleted);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFastForwardStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFastForwardCompleted);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimeScrubStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimeScrubCompleted);

// Tracks snapshots of actor state to support rewinding time
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class REWIND_API URewindComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// How often a snapshot should be recorded
	UPROPERTY(EditDefaultsOnly, Category = "Rewind")
	float SnapshotFrequencySeconds = 1.0f / 30.0f;

	// Whether actor requires movement component to be rewound
	UPROPERTY(EditDefaultsOnly, Category = "Rewind")
	bool bSnapshotMovementVelocityAndMode = false;

	// Whether actor should pause animations during time scrubbing
	UPROPERTY(EditDefaultsOnly, Category = "Rewind")
	bool bPauseAnimationDuringTimeScrubbing = false;

	// Called when the component begins time manipulation
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnTimeManipulationStarted OnTimeManipulationStarted;

	// Called when the component stops time manipulation
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnTimeManipulationCompleted OnTimeManipulationCompleted;

	// Called when the component begins rewinding
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnRewindStarted OnRewindStarted;

	// Called when the component stops rewinding
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnRewindCompleted OnRewindCompleted;

	// Called when the component begins fast forwarding
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnRewindStarted OnFastForwardStarted;

	// Called when the component stops fast forwarding
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnRewindCompleted OnFastForwardCompleted;

	// Called when the component begins rewinding
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnTimeScrubStarted OnTimeScrubStarted;

	// Called when the component stops rewinding
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnTimeScrubCompleted OnTimeScrubCompleted;

protected:
	// Whether the component is currently rewinding
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Rewind")
	bool bIsRewinding = false;

public:
	// Returns whether the component is currently rewinding
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	bool IsRewinding() const { return bIsRewinding; };

protected:
	// Whether the component is currently fast forwarding
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Rewind")
	bool bIsFastForwarding = false;

public:
	// Returns whether the component is currently fast forwarding
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	bool IsFastForwarding() const { return bIsFastForwarding; };

protected:
	// Whether the component is currently time scrubbing
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Rewind")
	bool bIsTimeScrubbing = false;

public:
	// Returns whether the component is currently time scrubbing
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	bool IsTimeScrubbing() const { return bIsTimeScrubbing; };

	// Returns whether the component is currently doing any form of time manipulation
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	bool IsTimeBeingManipulated() const { return bIsRewinding || bIsFastForwarding || bIsTimeScrubbing; };

private:
	// Whether rewinding is currently enabled
	UPROPERTY(VisibleAnywhere, Category = "Rewind")
	bool bIsRewindingEnabled = true;

public:
	// Returns whether rewinding is currently enabled
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	bool IsRewindingEnabled() const { return bIsRewindingEnabled; }

	// Sets whether rewinding is currently enabled
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	void SetIsRewindingEnabled(bool bEnabled);

public:
	// Sets default values for this component's properties
	URewindComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// State snapshots used when rewinding transforms and velocity
	struct FTransformAndVelocitySnapshot
	{
		// Time since the last snapshot was recorded
		float TimeSinceLastSnapshot = 0.0f;

		// Transform at time snapshot was recorded
		FTransform Transform{ FVector::ZeroVector };

		// Linear velocity from the owner's root primitive component at time snapshot was recorded
		FVector LinearVelocity = FVector::ZeroVector;

		// Angular velocity from the owner's root primitive component at time snapshot was recorded
		FVector AngularVelocityInRadians = FVector::ZeroVector;
	};

	// State snapshots used when rewinding movement
	struct FMovementVelocityAndModeSnapshot
	{
		// Time since the last snapshot was recorded
		float TimeSinceLastSnapshot = 0.0f;

		// Movement velocity from the owner's movement component at time snapshot was recorded
		FVector MovementVelocity = FVector::ZeroVector;

		// Movement mode from the owner's movement component at time snapshot was recorded
		TEnumAsByte<enum EMovementMode> MovementMode = EMovementMode::MOVE_None;
	};

	// Buffer storing transform and velocity snapshots for rewinding
	TRingBuffer<FTransformAndVelocitySnapshot> TransformAndVelocitySnapshots;

	// Buffer storing movement velocity and mode snapshots for rewinding
	TRingBuffer<FMovementVelocityAndModeSnapshot> MovementVelocityAndModeSnapshots;

	// Max snapshots to store; computed in BeginPlay
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	uint32 MaxSnapshots = 1;

	// Time since the last snapshot was added/removed to/from the ring buffer
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	float TimeSinceSnapshotsChanged = 0.0f;

	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	int32 LatestSnapshotIndex = -1;

	// Root primitive component on owner, if one exists
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	UPrimitiveComponent* OwnerRootComponent;

	// Movement component on owner, if one exists
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	UCharacterMovementComponent* OwnerMovementComponent;

	// Skeletal mesh component on owner, if one exists
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	USkeletalMeshComponent* OwnerSkeletalMesh;

	// Whether time manipulation paused physics
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	bool bPausedPhysics = false;

	// Whether time manipulation paused animation
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	bool bPausedAnimation = false;

	// Whether animation was paused when the current time manipulation operation began
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	bool bAnimationsPausedAtStartOfTimeManipulation = false;

	// Whether last manipulation was rewind or fast forward; used by time scrubbing to determine interpolation direction
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	bool bLastTimeManipulationWasRewind = true;

	// Game mode for global rewind state
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	ARewindGameMode* GameMode;

	// Color used for this actor in debug visualization when `Rewind.VisualizeSnapshots 1` is set
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	FColor DebugColor;

	// Called when rewinding starts
	UFUNCTION()
	void OnGlobalRewindStarted();

	// Called when fast forwarding starts
	UFUNCTION()
	void OnGlobalFastForwardStarted();

	// Called when time scrubbing starts
	UFUNCTION()
	void OnGlobalTimeScrubStarted();

	// Called when rewinding completes
	UFUNCTION()
	void OnGlobalRewindCompleted();

	// Called when fast forwarding completes
	UFUNCTION()
	void OnGlobalFastForwardCompleted();

	// Called when time scrubbing completes
	UFUNCTION()
	void OnGlobalTimeScrubCompleted();

	// Computes required space and initializes the ring buffers
	void InitializeRingBuffers(float MaxRewindSeconds);

	// Stores a snapshot in the ring buffer
	void RecordSnapshot(float DeltaTime);

	// Deletes all snapshots after the latest one
	void EraseFutureSnapshots();

	// Plays back and forth through time using the snapshots in the ring buffer
	void PlaySnapshots(float DeltaTime, bool bRewinding);

	// Advances to the next snapshot if rewinding or fast forwarding, then freezes time
	void PauseTime(float DeltaTime, bool bRewinding);

	// Helper to start a time manipulation operation
	bool TryStartTimeManipulation(bool& bStateToSet, bool bResetTimeSinceSnapshotsChanged);

	// Helper to stop a time manipulation operation
	bool TryStopTimeManipulation(bool& bStateToSet, bool bResetTimeSinceSnapshotsChanged, bool bResetMovementVelocity);

	// Disables physics
	void PausePhysics();

	// Recreates physics and movement state
	void UnpausePhysics();

	// Disables animation
	void PauseAnimation();

	// Resumes animation
	void UnpauseAnimation();

	// Helper function for PlaySnapshots/PauseTime that handle cases where there are insufficient snapshots to interpolate
	bool HandleInsufficientSnapshots();

	// Interpolates between the two latest snapshots and applies the result to the owner
	void InterpolateAndApplySnapshots(bool bRewinding);

	// Blends between two transform and velocity snapshots
	FTransformAndVelocitySnapshot BlendSnapshots(
		const FTransformAndVelocitySnapshot& A,
		const FTransformAndVelocitySnapshot& B,
		float Alpha);

	// Blends between two movement velocity and movement mode snapshots
	FMovementVelocityAndModeSnapshot BlendSnapshots(
		const FMovementVelocityAndModeSnapshot& A,
		const FMovementVelocityAndModeSnapshot& B,
		float Alpha);

	// Applies the provided transform and velocity snapshot to the owner
	void ApplySnapshot(const FTransformAndVelocitySnapshot& Snapshot, bool bApplyPhysics);

	// Applies the provided movement velocity and movement mode snapshot to the owner
	void ApplySnapshot(const FMovementVelocityAndModeSnapshot& Snapshot, bool bApplyTimeDilationToVelocity);

public:
	// Toggles debug visualization for snapshots
	static void ToggleTimelineSplinesVisibility();

private:
	// Debug helper to draw snapshots when `Rewind.VisualizeSnapshots 1` is set
	void DebugDrawSnapshots();
};
