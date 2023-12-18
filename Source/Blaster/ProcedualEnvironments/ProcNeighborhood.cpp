// Fill out your copyright notice in the Description page of Project Settings.

#include "ProcNeighborhood.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

const float AProcNeighborhood::CellSize = 100.f;

// Sets default values
AProcNeighborhood::AProcNeighborhood()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AProcNeighborhood::BeginPlay()
{
	Super::BeginPlay();
	InitializeGrid();
	GenerateRoads();
	SpawnFinishedNeighborhood();
}

void AProcNeighborhood::InitializeGrid()
{
	for (int32 Row = 0; Row < GridSize; ++Row)
	{
		for (int32 Col = 0; Col < GridSize; Col++)
		{
			Grid[Row][Col] = CellType::Empty;
		}
	}
}

void AProcNeighborhood::GenerateRoads()
{
	int32 Lifetime = FMath::RandRange(MinLifetime, MaxLifetime);
	int32 StartRow = FMath::RandRange(0, GridSize - 1);
	int32 StartCol = FMath::RandRange(0, GridSize - 1);
	UE_LOG(LogTemp, Log, TEXT("Seed ~ Lifetime: %i StartRow: %i StartCol: %i"), Lifetime, StartRow, StartCol);

	//this is sick, we use 0-3 irand to choose one of our enum options
	EDirection CurrentDirection = static_cast<EDirection>(FMath::RandRange(0, 3));
	PlaceRoad(StartRow, StartCol);

	for (int32 i = 0; i < Lifetime; i++)
	{
		if (FMath::RandRange(1, 100) <= BranchingFrequency)
		{
			//GenerateRoadBranch(StartRow, StartCol, Lifetime, CurrentDirection);
		}
		if (FMath::RandBool()) //50% chance cont straight
		{
			MoveInDirection(CurrentDirection, StartRow, StartCol);
		}
		else //25% chance left, and 25% right
		{
			ChangeDirection(CurrentDirection);
			MoveInDirection(CurrentDirection, StartRow, StartCol);
		}
	}
}

void AProcNeighborhood::GenerateRoadBranch(int32 StartRow, int32 StartCol, int32 Lifetime, EDirection CurrentDirection)
{
	//this is sick, we use 0-3 irand to choose one of our enum options
	ChangeDirection(CurrentDirection);
	for (int32 j = 0; j < Lifetime; j++)
	{
		if (FMath::RandRange(1, 100) <= BranchingFrequency)
		{
			GenerateRoadBranch(StartRow, StartCol, Lifetime, CurrentDirection);
		}
		if (FMath::RandBool()) //50% chance cont straight
		{
			MoveInDirection(CurrentDirection, StartRow, StartCol);
		}
		else //25% chance left, and 25% right
		{
			ChangeDirection(CurrentDirection);
			MoveInDirection(CurrentDirection, StartRow, StartCol);
		}
	}
}

void AProcNeighborhood::MoveInDirection(EDirection Direction, int32& Row, int32& Col)
{
	switch (Direction)
	{
	case EDirection::Up:
		Row--;
		break;
	case EDirection::Down:
		Row++;
		break;
	case EDirection::Left:
		Col--;
		break;
	case EDirection::Right:
		Col++;
		break;
	}

	Row = FMath::Clamp(Row, 0, GridSize - 1);
	Col = FMath::Clamp(Col, 0, GridSize - 1);

	PlaceRoad(Row, Col);
}

void AProcNeighborhood::ChangeDirection(EDirection& CurrentDirection)
{
	int32 RawDirection = static_cast<int32>(CurrentDirection);
	int32 LeftOrRight = FMath::RandBool() ? -1 : 1;
	int32 NextRawDirection = (RawDirection + LeftOrRight + 3) % 3;
	NextRawDirection = FMath::Clamp(NextRawDirection, 0, 3);//jus2bsafe
	//OutParameter
	CurrentDirection = static_cast<EDirection>(NextRawDirection);
}

void AProcNeighborhood::PlaceRoad(int32 Row, int32 Col)
{
	Grid[Row][Col] = CellType::Road;
}

void AProcNeighborhood::SpawnFinishedNeighborhood()
{
	for (int32 Row = 0; Row < GridSize; ++Row)
	{
		for (int32 Col = 0; Col < GridSize; Col++)
		{
			FVector SpawnLocation = GetActorLocation() + FVector(Row * CellSize, Col * CellSize, 0.0f);
			switch (Grid[Row][Col]) 
			{
			case CellType::Road:
				GetWorld()->SpawnActor<AActor>(RoadClass, SpawnLocation, FRotator::ZeroRotator);
			break;
			default:
				
			break;
			}
		}
	}	
}