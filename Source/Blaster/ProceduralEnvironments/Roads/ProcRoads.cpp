// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcRoads.h"

// Sets default values
AProcRoads::AProcRoads()
{
	PrimaryActorTick.bCanEverTick = false;
	RoadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoadMesh"));
	SetRootComponent(RoadMesh);
	RoadMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

	// Create and attach spawn point components
	// for (int32 i = 0; i < 8; ++i)
	// {
	// 	USceneComponent* NewSpawnPoint = CreateDefaultSubobject<USceneComponent>(FName("SpawnPoint" + FString::FromInt(i)));
	// 	NewSpawnPoint->SetupAttachment(RootComponent);
	// 	SpawnPoints.Add(NewSpawnPoint);
	// }
}

void AProcRoads::BeginPlay()
{
    Super::BeginPlay();
}

void AProcRoads::ProcGen()
{
    // if (SpawnableObjects.Num() == 0 || SpawnPoints.Num() == 0)
    //     return;
    //
    // // Random chance distribution
    // TArray<int32> ChanceDistribution = { 50, 40, 10, 5, 2, 1 }; // Represents 0, 1, 2, 3, 4, 5 spawns
    // int32 SpawnCount = ChooseSpawnCount(ChanceDistribution);
    //
    // TArray<USceneComponent*> ChosenSpawnPoints;
    // while (ChosenSpawnPoints.Num() < SpawnCount)
    // {
    //     USceneComponent* SelectedPoint = SpawnPoints[RS.RandRange(0, SpawnPoints.Num() - 1)];
    //     if (!ChosenSpawnPoints.Contains(SelectedPoint))
    //     {
    //         ChosenSpawnPoints.Add(SelectedPoint);
    //         SpawnObjectAtPoint(SelectedPoint);
    //     }
    // }
}

int32 AProcRoads::ChooseSpawnCount(const TArray<int32>& Distribution)
{
    int32 RandomNumber = RS.RandRange(1, 100);
    int32 AccumulatedChance = 0;

    for (int32 i = 0; i < Distribution.Num(); ++i)
    {
        AccumulatedChance += Distribution[i];
        if (RandomNumber <= AccumulatedChance)
            return i; // i represents the number of objects to spawn
    }

    return 0; // Default to 0 if no other condition met
}

void AProcRoads::SpawnObjectAtPoint(USceneComponent* SpawnPoint)
{
    if (SpawnableObjects.Num() == 0)
        return;

    TSubclassOf<AActor> SelectedObject = SpawnableObjects[RS.RandRange(0, SpawnableObjects.Num() - 1)];

    FVector Location = SpawnPoint->GetComponentLocation();
    FRotator Rotation = SpawnPoint->GetComponentRotation() + FRotator(0.f, RS.RandRange(-180, 180), 0.f);

    SpawnAt(SelectedObject, Location, Rotation);
}

