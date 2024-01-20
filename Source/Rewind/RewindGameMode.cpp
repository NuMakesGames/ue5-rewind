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
	TRACE_BOOKMARK(TEXT("ARewindGameMode::StartGlobalRewind"));

	bIsGlobalRewinding = true;
	OnGlobalRewindStarted.Broadcast();
}

void ARewindGameMode::StopGlobalRewind()
{
	TRACE_BOOKMARK(TEXT("ARewindGameMode::StopGlobalRewind"));

	bIsGlobalRewinding = false;
	OnGlobalRewindCompleted.Broadcast();
}

void ARewindGameMode::StartGlobalFastForward()
{
	TRACE_BOOKMARK(TEXT("ARewindGameMode::StartGlobalFastForward"));

	bIsGlobalFastForwarding = true;
	OnGlobalFastForwardStarted.Broadcast();
}

void ARewindGameMode::StopGlobalFastForward()
{
	TRACE_BOOKMARK(TEXT("ARewindGameMode::StopGlobalFastForward"));

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
	if (bIsGlobalTimeScrubbing)
	{
		TRACE_BOOKMARK(TEXT("ARewindGameMode::ToggleTimeScrub - Start Time Scrubbing"));
		OnGlobalTimeScrubStarted.Broadcast();
	}
	else
	{
		TRACE_BOOKMARK(TEXT("ARewindGameMode::ToggleTimeScrub - Stop Time Scrubbing"));
		OnGlobalTimeScrubCompleted.Broadcast();
	}
}

void ARewindGameMode::ToggleGlobalTimelineVisualization()
{
	bIsGlobalTimelineVisualizationEnabled = !bIsGlobalTimelineVisualizationEnabled;
	if (bIsGlobalTimelineVisualizationEnabled)
	{
		TRACE_BOOKMARK(TEXT("ARewindGameMode::ToggleGlobalTimelineVisualization - Enable Timeline Visualization"));
		OnGlobalTimelineVisualizationEnabled.Broadcast();
	}
	else
	{
		TRACE_BOOKMARK(TEXT("ARewindGameMode::ToggleGlobalTimelineVisualization - Disable Timeline Visualization"));
		OnGlobalTimelineVisualizationDisabled.Broadcast();
	}
}