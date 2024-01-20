// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"

#include "RewindCharacter.generated.h"

class ARewindGameMode;
struct FInputActionValue;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class URewindComponent;
class URewindVisualizationComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config = Game)
class ARewindCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Rewind component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rewind", meta = (AllowPrivateAccess = "true"))
	URewindComponent* RewindComponent;

	// Component for visualizing the rewind timeline
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rewind", meta = (AllowPrivateAccess = "true"))
	URewindVisualizationComponent* RewindVisualizationComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Time Scrub Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleTimeScrubAction;

	/** Rewind Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RewindAction;

	/** Fast Forward Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FastForwardAction;

	/** Set Rewind Speed Slowest Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SetRewindSpeedSlowestAction;

	/** Set Rewind Speed Slower Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SetRewindSpeedSlowerAction;

	/** Set Rewind Speed Normal Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SetRewindSpeedNormalAction;

	/** Set Rewind Speed Faster Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SetRewindSpeedFasterAction;

	/** Set Rewind Speed Fastest Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SetRewindSpeedFastestAction;

	/** Toggle Rewind Participation Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleRewindPartipationAction;

	/** Toggle Timeline Visualization Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleTimelineVisualizationAction;

public:
	// Desired camera arm length during rewinding
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewind")
	float RewindTargetCameraArmLength = 2000.0f;

private:
	// Original camera arm length; restored after rewind view
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind | Debug")
	float OriginalTargetCameraArmLength = 500.0f;

public:
	ARewindCharacter();

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for time scrub input */
	void ToggleTimeScrub(const FInputActionValue& Value);

	/** Called for rewind input */
	void Rewind(const FInputActionValue& Value);

	/** Called for rewind input */
	void StopRewinding(const FInputActionValue& Value);

	/** Called for fast forward input */
	void FastForward(const FInputActionValue& Value);

	/** Called for fast forward input */
	void StopFastForwarding(const FInputActionValue& Value);

	/** Called when setting rewind speed */
	void SetRewindSpeedSlowest(const FInputActionValue& Value);

	/** Called when setting rewind speed */
	void SetRewindSpeedSlower(const FInputActionValue& Value);

	/** Called when setting rewind speed */
	void SetRewindSpeedNormal(const FInputActionValue& Value);

	/** Called when setting rewind speed */
	void SetRewindSpeedFaster(const FInputActionValue& Value);

	/** Called when setting rewind speed */
	void SetRewindSpeedFastest(const FInputActionValue& Value);

	/** Called when toggling rewind participation */
	void ToggleRewindParticipation(const FInputActionValue& Value);

	/** Called when toggling timeline splines */
	void ToggleTimelineVisualization(const FInputActionValue& Value);

private:
	// Game mode for driving global time manipulation operations
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	ARewindGameMode* GameMode;

	// Updates camera POV based on rewind state
	UFUNCTION()
	void UpdateCamera();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	virtual void Jump() override;
	virtual void StopJumping() override;
};
