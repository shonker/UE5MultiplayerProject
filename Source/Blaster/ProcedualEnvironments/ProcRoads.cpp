// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcRoads.h"

// Sets default values
AProcRoads::AProcRoads()
{
	PrimaryActorTick.bCanEverTick = false;
	RoadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoadMesh"));
	SetRootComponent(RoadMesh);
	RoadMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
}

// Called when the game starts or when spawned
void AProcRoads::BeginPlay()
{
	Super::BeginPlay();
}


