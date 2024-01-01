// Fill out your copyright notice in the Description page of Project Settings.

#include "RewindableStaticMeshActor.h"

#include "Components/StaticMeshComponent.h"
#include "RewindComponent.h"

ARewindableStaticMeshActor::ARewindableStaticMeshActor()
{
	PrimaryActorTick.bCanEverTick = false;
	GetStaticMeshComponent()->Mobility = EComponentMobility::Movable;
	GetStaticMeshComponent()->SetSimulatePhysics(true);

	// Setup a rewind component that snapshots 30 times per second
	RewindComponent = CreateDefaultSubobject<URewindComponent>(TEXT("RewindComponent"));
	RewindComponent->SnapshotFrequencySeconds = 1.0f / 30.0f;
}
