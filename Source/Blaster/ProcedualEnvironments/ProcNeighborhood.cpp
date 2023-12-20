// Fill out your copyright notice in the Description page of Project Settings.

#include "ProcNeighborhood.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

const float AProcNeighborhood::CellSize = 100.f * 20.f;

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
	if (HasAuthority())
	{
		InitializeGrid();
		GenerateRoads();
		InferRoadTypesAndRotations();
		GenerateHouses();
		SpawnFinishedNeighborhood();
	}
}

void AProcNeighborhood::InitializeGrid()
{
	for (int32 Col = 0; Col < GridSize; ++Col)
	{
		for (int32 Row = 0; Row < GridSize; Row++)
		{
			GridCellTypes[Col][Row] = CellType::Empty;
		}
	}
}

void AProcNeighborhood::GenerateRoads()
{
	int32 Lifetime = FMath::RandRange(MinLifetime, MaxLifetime);
	int32 StartCol = GridSize/2;
	int32 StartRow = GridSize/2;
	UE_LOG(LogTemp, Display, TEXT("Seed ~ Lifetime: %i StartCol: %i StartRow: %i"), Lifetime, StartCol, StartRow);

	//this is sick, we use 0-3 irand to choose one of our enum options
	EDirection CurrentDirection = static_cast<EDirection>(FMath::RandRange(0, 3));
	PlaceRoad(StartCol, StartRow);

	for (int32 i = 0; i < Lifetime; Lifetime--)
	{
		if (FMath::RandRange(1, 100) <= BranchingFrequency)
		{
			UE_LOG(LogTemp, Log, TEXT("Branch ~ Lifetime: %i StartCol: %i StartRow: %i"), Lifetime, StartCol, StartRow);
			GenerateRoadBranch(StartCol, StartRow, Lifetime, CurrentDirection);
		}
		if (FMath::RandBool()) //50% chance cont straight
		{
			MoveInDirection(CurrentDirection, StartCol, StartRow);
		}
		else //25% chance left, and 25% right
		{
			ChangeDirection(CurrentDirection);
			MoveInDirection(CurrentDirection, StartCol, StartRow);
		}
	}
}

void AProcNeighborhood::GenerateRoadBranch(int32 StartCol, int32 StartRow, int32 Lifetime, EDirection CurrentDirection)
{
	//this is sick, we use 0-3 irand to choose one of our enum options
	ChangeDirection(CurrentDirection);
	for (int32 i = 0; i < Lifetime; Lifetime--)
	{
		if (FMath::RandRange(1, 100) <= BranchingFrequency)
		{
			UE_LOG(LogTemp, Log, TEXT("Branch ~ Lifetime: %i StartCol: %i StartRow: %i"), Lifetime, StartCol, StartRow);
			GenerateRoadBranch(StartCol, StartRow, Lifetime, CurrentDirection);
		}
		if (FMath::RandBool()) //50% chance cont straight
		{
			MoveInDirection(CurrentDirection, StartCol, StartRow);
		}
		else //25% chance left, and 25% right
		{
			ChangeDirection(CurrentDirection);
			MoveInDirection(CurrentDirection, StartCol, StartRow);
		}
	}
}

void AProcNeighborhood::MoveInDirection(EDirection Direction, int32& Col, int32& Row)
{

	//check if walking into an unsafe border area
	switch (Direction)
	{
	case EDirection::Up:
		if (Row - 1 <= 1) ChangeDirection(Direction);
		break;
	case EDirection::Down:
		if (Row + 1 >= GridSize - 2) ChangeDirection(Direction);
		break;
	case EDirection::Left:
		if (Col - 1 <= 1) ChangeDirection(Direction);
		break;
	case EDirection::Right:
		if (Col + 1 >= GridSize - 2) ChangeDirection(Direction);
		break;
	}

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

	Col = FMath::Clamp(Col, 1, GridSize - 2);
	Row = FMath::Clamp(Row, 1, GridSize - 2);

	PlaceRoad(Col, Row);
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

void AProcNeighborhood::PlaceRoad(int32 Col, int32 Row)
{
	GridCellTypes[Col][Row] = CellType::Road;
}

void AProcNeighborhood::InferRoadTypesAndRotations()
{
	for (int32 Col = 0; Col < GridSize; ++Col)
	{
		for (int32 Row = 0; Row < GridSize; Row++)
		{	
			
			if (GridCellTypes[Col][Row] == CellType::Road)
			{
				bool ConnectedRight = false;
				bool ConnectedDown = false;
				bool ConnectedLeft = false;
				bool ConnectedUp = false;
				
				if (Col - 1 >= 0) //Check left
				{
					if (GridCellTypes[Col - 1][Row] == CellType::Road) ConnectedLeft = true;
				}
				if (Col + 1 < GridSize) //Check right
				{
					if (GridCellTypes[Col + 1][Row] == CellType::Road) ConnectedRight = true;
				}
				if (Row - 1 >= 0) //Check up
				{
					if (GridCellTypes[Col][Row - 1] == CellType::Road) ConnectedUp = true;
				}
				if (Row + 1 < GridSize) //Check down
				{
					if (GridCellTypes[Col][Row + 1] == CellType::Road) ConnectedDown = true;
				}
				uint8 ConnectionCount = ConnectedRight + ConnectedLeft + ConnectedDown + ConnectedUp;
				
				GridRoadTypes[Col][Row] = static_cast<ERoadType>(ConnectionCount);

				if (ConnectionCount == 2)
				{
					if (!(ConnectedRight && ConnectedLeft) && !(ConnectedUp && ConnectedDown))
					{
						GridRoadTypes[Col][Row] = ERoadType::TwoWayTurn;
					}
				}

				//find rotations for each individual road type
				switch (GridRoadTypes[Col][Row])
				{ 
					case (ERoadType::DeadEnd):
						if (ConnectedRight) GridRotations[Col][Row] = CellRotation::Rotation_0DegRight; //break;
						if (ConnectedLeft) GridRotations[Col][Row] = CellRotation::Rotation_180DegLeft;// break;
						if (ConnectedDown) GridRotations[Col][Row] = CellRotation::Rotation_90DegDown;// break;
						if (ConnectedUp) GridRotations[Col][Row] = CellRotation::Rotation_270DegUp;// break;
					break;
					case (ERoadType::TwoWay):
						if (ConnectedRight) GridRotations[Col][Row] = CellRotation::Rotation_0DegRight;// break;
						if (ConnectedDown) GridRotations[Col][Row] = CellRotation::Rotation_90DegDown;// break;
					break;
					case (ERoadType::TwoWayTurn):
						if (ConnectedRight && ConnectedDown) GridRotations[Col][Row] = CellRotation::Rotation_0DegRight;// break;
						if (ConnectedDown && ConnectedLeft) GridRotations[Col][Row] = CellRotation::Rotation_90DegDown;// break;
						if (ConnectedLeft && ConnectedUp) GridRotations[Col][Row] = CellRotation::Rotation_180DegLeft;// break;
						if (ConnectedUp && ConnectedRight) GridRotations[Col][Row] = CellRotation::Rotation_270DegUp; //break;
					break;
					case (ERoadType::ThreeWay):
						if (ConnectedRight && ConnectedLeft)
						{
							if (ConnectedUp)
							{
								GridRotations[Col][Row] = CellRotation::Rotation_180DegLeft;
								break;
							}
							if (ConnectedDown)
							{
								GridRotations[Col][Row] = CellRotation::Rotation_0DegRight;
								break;
							}
						}
						if (ConnectedUp && ConnectedDown)
						{
							if (ConnectedRight)
							{
								GridRotations[Col][Row] = CellRotation::Rotation_270DegUp;
								break;
							}
							if (ConnectedLeft)
							{
								GridRotations[Col][Row] = CellRotation::Rotation_90DegDown;
								break;
							}
						}
					break;
					case (ERoadType::FourWay):
						GridRotations[Col][Row] = FMath::RandBool() ? 
								(FMath::RandBool() ? CellRotation::Rotation_0DegRight : CellRotation::Rotation_270DegUp):
								(FMath::RandBool() ? CellRotation::Rotation_90DegDown : CellRotation::Rotation_180DegLeft);
					break;
					default: //shouldnt happen
						GridRotations[Col][Row] = CellRotation::Rotation_0DegRight;
						GridCellTypes[Col][Row] = CellType::Empty;
						UE_LOG(LogTemp, Error, TEXT("Default GridCell Rotation Should Not Be Selected"));
					break;
				}
			}
		}
	}
}

//generated in empty cells touching roads
void AProcNeighborhood::GenerateHouses()
{
	TMap<FVector2D, uint8> ConnectionCounts;

	for (int32 Col = 0; Col < GridSize; ++Col)
	{
		for (int32 Row = 0; Row < GridSize; Row++)
		{
			if (GridCellTypes[Col][Row] == CellType::Empty)
			{
				bool ConnectedRight = false;
				bool ConnectedDown = false;
				bool ConnectedLeft = false;
				bool ConnectedUp = false;

				if (Col - 1 >= 0) //Check left
				{
					if (GridCellTypes[Col - 1][Row] == CellType::Road) ConnectedLeft = true;
				}
				if (Col + 1 < GridSize) //Check right
				{
					if (GridCellTypes[Col + 1][Row] == CellType::Road) ConnectedRight = true;
				}
				if (Row - 1 >= 0) //Check up
				{
					if (GridCellTypes[Col][Row - 1] == CellType::Road) ConnectedUp = true;
				}
				if (Row + 1 < GridSize) //Check down
				{
					if (GridCellTypes[Col][Row + 1] == CellType::Road) ConnectedDown = true;
				}

				uint8 ConnectionCount = ConnectedRight + ConnectedLeft + ConnectedDown + ConnectedUp;

				ConnectionCounts.Add(FVector2D(Col, Row), ConnectionCount);


				TArray<CellRotation> PossibleRotations;

				if (ConnectedLeft) PossibleRotations.Add(CellRotation::Rotation_180DegLeft);
				if (ConnectedRight) PossibleRotations.Add(CellRotation::Rotation_0DegRight);
				if (ConnectedUp) PossibleRotations.Add(CellRotation::Rotation_270DegUp);
				if (ConnectedDown) PossibleRotations.Add(CellRotation::Rotation_90DegDown);

				if (PossibleRotations.Num() > 0)
				{
					int32 RandomIndex = FMath::RandRange(0, PossibleRotations.Num() - 1);
					GridRotations[Col][Row] = PossibleRotations[RandomIndex];
				}
			}
		}
	}

	uint8 HouseCount = FMath::RandRange(MinHouseCount, MaxHouseCount);
	for (const auto& Entry : ConnectionCounts)
	{
		if (HouseCount <= 0) continue;
		FVector2D CandidateLocation = Entry.Key;
		uint8 ConnectionCount = Entry.Value;
		int32 Col = CandidateLocation.X;
		int32 Row = CandidateLocation.Y;
		
		float Chance = ConnectionCount * FMath::RandRange(0, 50);

		if (Chance > 40)
		{
			GridCellTypes[Col][Row] = CellType::House;
			HouseCount -= 1;
		}
	}
}


void AProcNeighborhood::SpawnFinishedNeighborhood()
{
	for (int32 Col = 0; Col < GridSize; ++Col)
	{
		for (int32 Row = 0; Row < GridSize; Row++)
		{
			TSubclassOf<AActor> RoadBlueprint = nullptr;
			FVector SpawnLocation = GetActorLocation() + FVector(Col * CellSize, Row * CellSize, 0.0f);
			UWorld* World = GetWorld();
			if (World)
			{
				switch (GridCellTypes[Col][Row])
				{
				case CellType::Road:
					switch (GridRoadTypes[Col][Row])
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
						RoadBlueprint = TwoWayTurnBlueprintClass;
						break;
					default:
						RoadBlueprint = DeadEndBlueprint;
						break;
					}

					if (RoadBlueprint)
					{
						FRotator RoadRotation = FRotator(0.0f, static_cast<float>(GridRotations[Col][Row]), 0.0f);
						AActor* SpawnedRoad = GetWorld()->SpawnActor<AActor>(RoadBlueprint, SpawnLocation, RoadRotation);
						if (SpawnedRoad)
						{
							SpawnedRoads.Add(SpawnedRoad);
						}
					}
					break;
				case CellType::House:
					if (World)
					{
						if (HouseBlueprintClass)
						{
							FRotator HouseRotation = FRotator(0.0f, static_cast<float>(GridRotations[Col][Row]), 0.0f);
							AActor* SpawnedHouse = GetWorld()->SpawnActor<AActor>(HouseBlueprintClass, SpawnLocation, HouseRotation);
							if (SpawnedHouse)
							{
								SpawnedHouses.Add(SpawnedHouse);
							}
						}
					}
				}
			}
		}
	}	
}