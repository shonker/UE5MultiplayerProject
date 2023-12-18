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
	InferRoadTypesAndRotations();
	SpawnFinishedNeighborhood();
}

void AProcNeighborhood::InitializeGrid()
{
	for (int32 Row = 0; Row < GridSize; ++Row)
	{
		for (int32 Col = 0; Col < GridSize; Col++)
		{
			GridCellTypes[Row][Col] = CellType::Empty;
		}
	}
}

void AProcNeighborhood::GenerateRoads()
{
	int32 Lifetime = FMath::RandRange(MinLifetime, MaxLifetime);
	int32 StartRow = GridSize/2;
	int32 StartCol = GridSize/2;
	UE_LOG(LogTemp, Display, TEXT("Seed ~ Lifetime: %i StartRow: %i StartCol: %i"), Lifetime, StartRow, StartCol);

	//this is sick, we use 0-3 irand to choose one of our enum options
	EDirection CurrentDirection = static_cast<EDirection>(FMath::RandRange(0, 3));
	PlaceRoad(StartRow, StartCol);

	for (int32 i = 0; i < Lifetime; Lifetime--)
	{
		if (FMath::RandRange(1, 100) <= BranchingFrequency)
		{
			UE_LOG(LogTemp, Log, TEXT("Branch ~ Lifetime: %i StartRow: %i StartCol: %i"), Lifetime, StartRow, StartCol);
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

void AProcNeighborhood::GenerateRoadBranch(int32 StartRow, int32 StartCol, int32 Lifetime, EDirection CurrentDirection)
{
	//this is sick, we use 0-3 irand to choose one of our enum options
	ChangeDirection(CurrentDirection);
	for (int32 i = 0; i < Lifetime; Lifetime--)
	{
		if (FMath::RandRange(1, 100) <= BranchingFrequency)
		{
			UE_LOG(LogTemp, Log, TEXT("Branch ~ Lifetime: %i StartRow: %i StartCol: %i"), Lifetime, StartRow, StartCol);
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
	GridCellTypes[Row][Col] = CellType::Road;
}

void AProcNeighborhood::InferRoadTypesAndRotations()
{
	for (int32 Row = 0; Row < GridSize; ++Row)
	{
		for (int32 Col = 0; Col < GridSize; Col++)
		{	
			
			if (GridCellTypes[Row][Col] == CellType::Road)
			{
				bool ConnectedRight = false;
				bool ConnectedDown = false;
				bool ConnectedLeft = false;
				bool ConnectedUp = false;
				
				if (Col - 1 >= 0) //Check left
				{
					if (GridCellTypes[Row][Col - 1] == CellType::Road) ConnectedLeft = true;
				}
				if (Col + 1 < GridSize) //Check right
				{
					if (GridCellTypes[Row][Col + 1] == CellType::Road) ConnectedRight = true;
				}
				if (Row - 1 >= 0) //Check up
				{
					if (GridCellTypes[Row - 1][Col] == CellType::Road) ConnectedUp = true;
				}
				if (Row + 1 < GridSize) //Check down
				{
					if (GridCellTypes[Row + 1][Col] == CellType::Road) ConnectedDown = true;
				}
				uint8 ConnectionCount = ConnectedRight + ConnectedLeft + ConnectedDown + ConnectedUp;
				GridRoadTypes[Row][Col] = static_cast<ERoadType>(ConnectionCount);

				//Straightaway or Turn?
				if (GridRoadTypes[Row][Col] == ERoadType::TwoWay)
				{
					if (!(ConnectedRight && ConnectedLeft) && !(ConnectedUp && ConnectedDown))
					{
						GridRoadTypes[Row][Col] = ERoadType::TwoWayTurn;
					}
				}

				//find rotations for each individual road type
				switch (GridRoadTypes[Row][Col])
				{ 
					case (ERoadType::DeadEnd):
						if (ConnectedRight) GridRotations[Row][Col] = CellRotation::Rotation_0DegRight; break;
						if (ConnectedLeft) GridRotations[Row][Col] = CellRotation::Rotation_180DegLeft; break;
						if (ConnectedDown) GridRotations[Row][Col] = CellRotation::Rotation_90DegDown; break;
						if (ConnectedUp) GridRotations[Row][Col] = CellRotation::Rotation_270DegUp; break;
						UE_LOG(LogTemp, Error, TEXT("Logic Problem in DeadEnd Rotation"));
					break;
					case (ERoadType::TwoWay):
						if (ConnectedRight) GridRotations[Row][Col] = CellRotation::Rotation_0DegRight; break;
						if (ConnectedDown) GridRotations[Row][Col] = CellRotation::Rotation_90DegDown; break;
						UE_LOG(LogTemp, Error, TEXT("Logic Problem in TwoWay Rotation"));

					break;
					case (ERoadType::TwoWayTurn):
						if (ConnectedRight && ConnectedDown) GridRotations[Row][Col] = CellRotation::Rotation_0DegRight; break;
						if (ConnectedDown && ConnectedLeft) GridRotations[Row][Col] = CellRotation::Rotation_90DegDown; break;
						if (ConnectedLeft && ConnectedUp) GridRotations[Row][Col] = CellRotation::Rotation_180DegLeft; break;
						if (ConnectedUp && ConnectedRight) GridRotations[Row][Col] = CellRotation::Rotation_270DegUp; break;
						UE_LOG(LogTemp, Error, TEXT("Logic Problem in TwoWayTurn Rotation"));
					break;
					case (ERoadType::ThreeWay):
						if (ConnectedRight && ConnectedLeft)
						{
							if (ConnectedUp)
							{
								GridRotations[Row][Col] = CellRotation::Rotation_180DegLeft;
								break;
							}
							if (ConnectedDown)
							{
								GridRotations[Row][Col] = CellRotation::Rotation_0DegRight;
								break;
							}
						}
						if (ConnectedUp && ConnectedDown)
						{
							if (ConnectedRight)
							{
								GridRotations[Row][Col] = CellRotation::Rotation_270DegUp;
								break;
							}
							if (ConnectedLeft)
							{
								GridRotations[Row][Col] = CellRotation::Rotation_90DegDown;
								break;
							}
						}
						UE_LOG(LogTemp, Error, TEXT("Logic Problem in ThreeWay Rotation"));
					break;
					case (ERoadType::FourWay):
						GridRotations[Row][Col] = CellRotation::Rotation_0DegRight;
					break;
					default: //shouldnt happen
						GridRotations[Row][Col] = CellRotation::Rotation_0DegRight;
						UE_LOG(LogTemp, Error, TEXT("Default GridCell Rotation Should Not Be Selected"));
					break;
				}
			}
		}
	}
}

void AProcNeighborhood::SpawnFinishedNeighborhood()
{
	for (int32 Row = 0; Row < GridSize; ++Row)
	{
		for (int32 Col = 0; Col < GridSize; Col++)
		{
			FVector SpawnLocation = GetActorLocation() + FVector(Row * CellSize, Col * CellSize, 0.0f);
			AActor* SpawnedRoad;
			switch (GridCellTypes[Row][Col]) 
			{
			case CellType::Road:
				UWorld* World = GetWorld();
				if (World)
				{
					TSubclassOf<AActor> RoadBlueprint = nullptr;
					switch (GridRoadTypes[Row][Col])
					{
					case ERoadType::DeadEnd:
						RoadBlueprint = DeadEndBlueprint;
						break;
					case ERoadType::TwoWay:
						RoadBlueprint = TwoWayBlueprint;
						break;
					case ERoadType::ThreeWay:
						RoadBlueprint = ThreeWayBlueprint;
						break;
					case ERoadType::FourWay:
						RoadBlueprint = FourWayBlueprint;
						break;
					case ERoadType::TwoWayTurn:
						RoadBlueprint = TwoWayTurnBlueprint;
						break;
					default:
						RoadBlueprint = DeadEndBlueprint;
						break;
					}

					if (RoadBlueprint)
					{
						FRotator RoadRotation = FRotator(0.0f, 0.0f, static_cast<float>(GridRotations[Row][Col]));
						SpawnedRoad = GetWorld()->SpawnActor<AActor>(RoadClass, SpawnLocation, RoadRotation);
						if (SpawnedRoad)
						{
							SpawnedRoads.Add(SpawnedRoad);
						}
					}
				}
			break;
			}
		}
	}	
}