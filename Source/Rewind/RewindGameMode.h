// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/GameModeBase.h"

#include "RewindGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGlobalRewindStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGlobalRewindCompleted);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGlobalFastForwardStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGlobalFastForwardCompleted);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGlobalTimeScrubStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGlobalTimeScrubCompleted);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGlobalTimelineVisualizationEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGlobalTimelineVisualizationDisabled);

UCLASS(minimalapi)
class ARewindGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARewindGameMode();

	// Slowest time dilation speed for time manipulation operations
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rewind")
	float SlowestRewindSpeed = 0.25f;

	// Slower time dilation speed for time manipulation operations
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rewind")
	float SlowerRewindSpeed = 0.5f;

	// Normal time dilation speed for time manipulation operations
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rewind")
	float NormalRewindSpeed = 1.0f;

	// Faster time dilation speed for time manipulation operations
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rewind")
	float FasterRewindSpeed = 2.0f;

	// Fastest time dilation speed for time manipulation operations
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rewind")
	float FastestRewindSpeed = 4.0f;

	// Starts rewinding all actors with rewind components
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	void StartGlobalRewind();

	// Stops rewinding all actors with rewind components
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	void StopGlobalRewind();

	// Starts fast forwarding all actors with rewind components
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	void StartGlobalFastForward();

	// Stops fast forwarding all actors with rewind components
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	void StopGlobalFastForward();

	/** Called when setting rewind speed */
	void SetRewindSpeedSlowest();

	/** Called when setting rewind speed */
	void SetRewindSpeedSlower();

	/** Called when setting rewind speed */
	void SetRewindSpeedNormal();

	/** Called when setting rewind speed */
	void SetRewindSpeedFaster();

	/** Called when setting rewind speed */
	void SetRewindSpeedFastest();

	// Toggles time scrubbing on all actors with rewind components
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	void ToggleTimeScrub();

	// Toggles visualization of timeline on all actors with rewind components
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	void ToggleGlobalTimelineVisualization();

	// Event for when global rewinds start
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnGlobalRewindStarted OnGlobalRewindStarted;

	// Event for when global rewinds stop
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnGlobalRewindCompleted OnGlobalRewindCompleted;

	// Event for when global fast forwards start
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnGlobalFastForwardStarted OnGlobalFastForwardStarted;

	// Event for when global fast forwards stop
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnGlobalFastForwardCompleted OnGlobalFastForwardCompleted;

	// Event for when global time scrubbing starts
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnGlobalTimeScrubStarted OnGlobalTimeScrubStarted;

	// Event for when global time scrubbing stops
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnGlobalTimeScrubCompleted OnGlobalTimeScrubCompleted;

	// Event for when global timeline visualization is enabled
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnGlobalTimelineVisualizationEnabled OnGlobalTimelineVisualizationEnabled;

	// Event for when global timeline visualization is disabled
	UPROPERTY(BlueprintAssignable, Category = "Rewind")
	FOnGlobalTimelineVisualizationDisabled OnGlobalTimelineVisualizationDisabled;

	// Desired length of longest rewind; used to compute the rewind buffer size
	UPROPERTY(EditDefaultsOnly, Category = "Rewind")
	float MaxRewindSeconds = 120.0f;

private:
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind")
	bool bIsGlobalTimeScrubbing = false;

public:
	// Returns whether the component is currently rewinding
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	bool IsGlobalTimeScrubbing() const { return bIsGlobalTimeScrubbing; };

private:
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind")
	bool bIsGlobalTimelineVisualizationEnabled = false;

public:
	// Returns whether the timeline visualization is currently enabled
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	bool IsGlobalTimelineVisualizationEnabled() const { return bIsGlobalTimelineVisualizationEnabled; };

private:
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind")
	bool bIsGlobalRewinding = false;

public:
	// Returns whether rewinding is currently enabled
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	bool IsGlobalRewinding() const { return bIsGlobalRewinding; };

private:
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind")
	bool bIsGlobalFastForwarding = false;

public:
	// Returns whether fast forwarding is currently enabled
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	bool IsGlobalFastForwarding() const { return bIsGlobalFastForwarding; };

private:
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind")
	float GlobalRewindSpeed = 1.0f;

public:
	// Returns the current global time dilation
	UFUNCTION(BlueprintCallable, Category = "Rewind")
	float GetGlobalRewindSpeed() const { return GlobalRewindSpeed; }

};
