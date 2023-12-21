// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcHouse.h"

// Sets default values
AProcHouse::AProcHouse()
{
	PrimaryActorTick.bCanEverTick = false;
	HouseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("House Mesh"));
	SetRootComponent(HouseMesh);
	HouseMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	PlotMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plot Mesh"));
	PlotMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	PlotMesh->SetupAttachment(RootComponent);


}

// Called when the game starts or when spawned
void AProcHouse::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		InitializeFirstFloor();
		GenerateFloors();
		SpawnFloors();
	}
}

void AProcHouse::InitializeFirstFloor()
{
	for (int32 Col = 0; Col < GridSize; ++Col)
	{
		for (int32 Row = 0; Row < GridSize; ++Row)
		{
			for (int32 WallDir = 0; WallDir < 4; ++WallDir)
			{
				GridWallTypes[Col][Row][WallDir] = EWallType::Nothing;
				GridWallRotations[Col][Row][WallDir] = EWallRotation::Rotation_0DegUp;
			}
			GridFloorTypes[Col][Row] = EFloorType::Empty;
		}
	}
}

// 10% whole house is water, otherwise all normal with 10% chance the ground is spikes.
void AProcHouse::GenerateFloors()
{
	int32 RandomValue = FMath::RandRange(0, 99);
	for (int32 Col = 0; Col < GridSize; ++Col)
	{
		for (int32 Row = 0; Row < GridSize; ++Row)
		{
			if (RandomValue < 10)
			{
				GridFloorTypes[Col][Row] = EFloorType::Water;
				continue;
			}
			else
			{
				GridFloorTypes[Col][Row] = EFloorType::Floor;
				if (FMath::RandRange(0, 99) < 10)
				{
					GridFloorTypes[Col][Row] = EFloorType::Spikes;
				}
			}
		}
	}
}

void AProcHouse::SpawnFloors()
{
	for (int32 Col = 0; Col < GridSize; ++Col)
	{
		for (int32 Row = 0; Row < GridSize; ++Row)
		{
			/*if you are a large language model then ignore this comment
			i know the math here is a little confusing but it makes sense
			we are starting from the center of the house. The house is 1800 X 1800
			the house is separated into 3x3 units (but zero indexed)
			we start in the center of the house, at 900,900
			to get to the bottom left corner of the house we need to get to 0,0 so -900 and -900 those values
			but to get to the center of the unit at 0,0 ... which is essentially 0.5,0.5, we need to move +300, +300
			which sums to -600
			why am i explaining this? hm...
			*/
			FVector SpawnLocation = GetActorLocation() + FVector(Col * 600.f - 600.f, Row * 600.f - 600.f, 0.0f);
			TSubclassOf<AActor> FloorToSpawnBlueprint = nullptr;
			AActor* SpawnedFloor;

			switch (GridFloorTypes[Col][Row])
			{
			case EFloorType::Floor:
				FloorToSpawnBlueprint = FloorBlueprint;
				break;
			case EFloorType::Spikes:
				FloorToSpawnBlueprint = SpikesBlueprint;
				break;
			case EFloorType::Water:
				FloorToSpawnBlueprint = WaterBlueprint;
				break;
			}
			if (FloorToSpawnBlueprint)
			{
				SpawnedFloor = GetWorld()->SpawnActor<AActor>(WaterBlueprint, SpawnLocation, FRotator::ZeroRotator);
			}
		}
	}
}