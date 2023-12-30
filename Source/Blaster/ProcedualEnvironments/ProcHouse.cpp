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
		//awful wall generation... I do not approve of this
		GenerateWalls();
		SpawnWalls();
	}
}

void AProcHouse::InitializeFirstFloor()
{
	for (int32 Col = 0; Col < GridWidth; ++Col)
	{
		for (int32 Row = 0; Row < GridHeight; ++Row)
		{
			for (int32 WallDir = 0; WallDir < 3; ++WallDir)
			{
				GridWallTypes[Col][Row][WallDir] = EWallType::Initialized;			}
			GridFloorTypes[Col][Row] = EFloorType::Empty;
		}
	}
}

void AProcHouse::GenerateWalls()
{
	FindUsableWalls();
	CreatePathways();
	AssignDoorways();
}

void AProcHouse::AssignDoorways()
{
	for (int32 Col = 0; Col < GridWidth; ++Col)
	{
		for (int32 Row = 0; Row < GridHeight; ++Row)
		{
			for (int32 WallDir = 0; WallDir < 3; ++WallDir)
			{
				if (GridWallTypes[Col][Row][WallDir] == EWallType::Wall)
				{
					if (FMath::RandRange(0, 100) < 20)
					{
						GridWallTypes[Col][Row][WallDir] = EWallType::Doorway;
					}
					GridWallTypes[1][0][2] = EWallType::FrontDoor;
				}
			}
		}
	}
}

void AProcHouse::CreatePathways()
{
	//choose random starting room and lifetime
	uint8 Lifetime = FMath::RandRange(2, 5);
	uint8 InitCol = FMath::RandRange(0, GridHeight);
	uint8 InitRow = FMath::RandRange(0, GridWidth);
	EPathDirection InitDirection = static_cast<EPathDirection>(FMath::RandRange(0, 3));

	EPathDirection Direction = InitDirection;
	EPathDirection LastDirection = Direction;
	uint8 Col = InitCol;
	uint8 Row = InitRow;

	for (uint8 i = 0; i < Lifetime; Lifetime--)
	{
		uint8 iDirection = static_cast<uint8>(Direction);
		uint8 iLastDirection = static_cast<uint8>(LastDirection);
		//direction of movement
		GridWallTypes[Col][Row][iDirection] = EWallType::Nothing;
		//behind direction
		//GridWallTypes[Col][Row][(iDirection + 3 - 2) % 3] = EWallType::Wall;
		//left and right of direction
		if (GridWallTypes[Col][Row][(iDirection + 3 - 1) % 3] == EWallType::Nothing)
		{
			GridWallTypes[Col][Row][(iDirection + 3 - 1) % 3] = EWallType::Wall;
		}

		if (GridWallTypes[Col][Row][(iDirection + 3 + 1) % 3] == EWallType::Nothing)
		{
			GridWallTypes[Col][Row][(iDirection + 3 + 1) % 3] = EWallType::Wall;
		}

		if (GridWallTypes[Col][Row][iDirection] != EWallType::Window)
		{
			GridWallTypes[Col][Row][iDirection] = EWallType::NotWall;
		}

		switch (Direction)
		{
		case EPathDirection::Up:
			if (Row < GridHeight) Row++;
			break;
		case EPathDirection::Down:
			if (Row > 0) Row--;
			break;
		case EPathDirection::Right:
			if (Col < GridWidth) Col++;
			break;
		case EPathDirection::Left:
			if (Col > 0) Col--;
			break;
		}
	}
	Direction = static_cast<EPathDirection>(FMath::RandRange(0, 3));


}

void AProcHouse::FindUsableWalls()
{
	//start by making the walls "nothing" from initialized if they are to be made into any variant of walls at all
	//and windows if they are to be later randomized as windows
	for (uint8 Col = 0; Col < GridWidth; ++Col)
	{
		for (uint8 Row = 0; Row < GridHeight; ++Row)
		{
			for (uint8 WallDir = 0; WallDir < 3; ++WallDir)
			{
				bool LookingAtWindow = (Col == GridWidth && WallDir == 1 //right
					|| Col == 0 && WallDir == 3 //left
					|| Row == 0 && WallDir == 2//bottom
					|| Row == GridHeight && WallDir == 0 //top
					);
				if (LookingAtWindow)
				{
					GridWallTypes[Col][Row][WallDir] = EWallType::Window;
					continue;
				}
				//now find what non window walls we will be dealing with that have not been approved for wall use
				//nothing == approved
				switch (WallDir)
				{
				case 0: //looking up
					if (GridWallTypes[Col][Row + 1][2] == EWallType::Initialized)
					{
						GridWallTypes[Col][Row][WallDir] = EWallType::Nothing;
					}
					break;
				case 1: //looking right
					if (GridWallTypes[Col + 1][Row][3] == EWallType::Initialized)
					{
						GridWallTypes[Col][Row][WallDir] = EWallType::Nothing;
					}
					break;
				case 2://looking down
					if (GridWallTypes[Col][Row - 1][0] == EWallType::Initialized)
					{
						GridWallTypes[Col][Row][WallDir] = EWallType::Nothing;
					}
				case 3: //looking left
					if (GridWallTypes[Col - 1][Row][1] == EWallType::Initialized)
					{
						GridWallTypes[Col][Row][WallDir] = EWallType::Nothing;
					}
				}
				//ok now we will NOT change anything that remains as "initialized" I stfg
			}
		}
	}
}

// 10% whole house is water, otherwise all normal with 10% chance the ground is spikes.
void AProcHouse::GenerateFloors()
{
	uint8 RandomValue = FMath::RandRange(0, 99);
	for (uint8 Col = 0; Col < GridWidth; ++Col)
	{
		for (uint8 Row = 0; Row < GridHeight; ++Row)
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
	for (uint8 Col = 0; Col < GridWidth; ++Col)
	{
		for (uint8 Row = 0; Row < GridHeight; ++Row)
		{
			/*
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

void AProcHouse::SpawnWalls()
{
	for (uint8 Col = 0; Col < GridWidth; ++Col)
	{
		for (uint8 Row = 0; Row < GridHeight; ++Row)
		{
			for (uint8 WallDir = 0; WallDir < 3; ++WallDir)
			{
				FVector SpawnLocation = GetActorLocation() + FVector(Col * 600.f - 600.f, Row * 600.f - 600.f, 0.0f);
				TSubclassOf<AActor> WallToSpawnBlueprint = nullptr;
				switch (GridWallTypes[Col][Row][WallDir])
				{
				case EWallType::Window:
					WallToSpawnBlueprint = WindowBlueprint;
					break;
				case EWallType::Wall:
					WallToSpawnBlueprint = WallBlueprint;
					break;
				case EWallType::Doorway:
					WallToSpawnBlueprint = DoorwayBlueprint;
					break;
				}
				if (WallToSpawnBlueprint)
				{
					FRotator WallRotation = FRotator(0.0f, static_cast<float>(GridWallTypes[Col][Row][WallDir]) * 90.f, 0.0f);
					AActor* SpawnedRoad = GetWorld()->SpawnActor<AActor>(WallToSpawnBlueprint, SpawnLocation, WallRotation);
				}
			}
		}
	}
}