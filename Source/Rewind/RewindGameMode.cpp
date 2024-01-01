// Copyright Epic Games, Inc. All Rights Reserved.

#include "RewindGameMode.h"

#include "RewindCharacter.h"
#include "UObject/ConstructorHelpers.h"

ARewindGameMode::ARewindGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL) { DefaultPawnClass = PlayerPawnBPClass.Class; }
}

void ARewindGameMode::StartGlobalRewind()
{
	bIsGlobalRewinding = true;
	OnGlobalRewindStarted.Broadcast();
}

void ARewindGameMode::StopGlobalRewind()
{
	bIsGlobalRewinding = false;
	OnGlobalRewindCompleted.Broadcast();
}

void ARewindGameMode::StartGlobalFastForward()
{
	bIsGlobalFastForwarding = true;
	OnGlobalFastForwardStarted.Broadcast();
}

void ARewindGameMode::StopGlobalFastForward()
{
	bIsGlobalFastForwarding = false;
	OnGlobalFastForwardCompleted.Broadcast();
}

void ARewindGameMode::SetRewindSpeedSlowest()
{
	GlobalRewindSpeed = SlowestRewindSpeed;
}

void ARewindGameMode::SetRewindSpeedSlower()
{
	GlobalRewindSpeed = SlowerRewindSpeed;
}

void ARewindGameMode::SetRewindSpeedNormal()
{
	GlobalRewindSpeed = NormalRewindSpeed;
}

void ARewindGameMode::SetRewindSpeedFaster()
{
	GlobalRewindSpeed = FasterRewindSpeed;
}

void ARewindGameMode::SetRewindSpeedFastest()
{
	GlobalRewindSpeed = FastestRewindSpeed;
}

void ARewindGameMode::ToggleTimeScrub()
{
	bIsGlobalTimeScrubbing = !bIsGlobalTimeScrubbing;
	if (bIsGlobalTimeScrubbing) { OnGlobalTimeScrubStarted.Broadcast(); }
	else { OnGlobalTimeScrubCompleted.Broadcast(); }
}