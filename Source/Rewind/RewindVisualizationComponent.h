// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Runtime/Core/Public/Containers/RingBuffer.h"

#include "RewindVisualizationComponent.generated.h"

struct FTransformAndVelocitySnapshot;

/**
 * Draws static mesh instances for each snapshot on the rewind timeline
 */
UCLASS()
class REWIND_API URewindVisualizationComponent : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	URewindVisualizationComponent();

public:
	// Clear all instances
	virtual void ClearInstances() override;

	// Assigns a static mesh to each transform in snapshots
	void SetInstancesFromSnapshots(const TRingBuffer<FTransformAndVelocitySnapshot>& Snapshots);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Time in seconds between each visualized mesh
	UPROPERTY(EditDefaultsOnly, Category = "Rewind|Visualization")
	float SecondsPerMesh = 1.0f / 30.0f;

private:
	// Color used for this actor in debug visualization when `Rewind.VisualizeSnapshots 1` is set
	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	FColor DebugColor;

	UPROPERTY(Transient, VisibleAnywhere, Category = "Rewind|Debug")
	float LastUpdateTime = 0.0f;
};
