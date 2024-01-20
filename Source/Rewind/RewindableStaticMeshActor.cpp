// Fill out your copyright notice in the Description page of Project Settings.

#include "RewindableStaticMeshActor.h"

#include "Components/StaticMeshComponent.h"
#include "RewindComponent.h"
#include "RewindVisualizationComponent.h"

ARewindableStaticMeshActor::ARewindableStaticMeshActor()
{
	PrimaryActorTick.bCanEverTick = false;
	GetStaticMeshComponent()->Mobility = EComponentMobility::Movable;
	GetStaticMeshComponent()->SetSimulatePhysics(true);

	// Setup a rewind component that snapshots 30 times per second
	RewindComponent = CreateDefaultSubobject<URewindComponent>(TEXT("RewindComponent"));
	RewindComponent->SnapshotFrequencySeconds = 1.0f / 30.0f;

	// Setup a rewind visualization component that draws a static mesh instance for each snapshot
	RewindVisualizationComponent = CreateDefaultSubobject<URewindVisualizationComponent>(TEXT("RewindVisualizationComponent"));
	RewindVisualizationComponent->SetupAttachment(RootComponent);
}
