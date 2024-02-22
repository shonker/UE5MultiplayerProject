// Fill out your copyright notice in the Description page of Project Settings.

#include "ProcNeighborhood.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "AProcActor.h"
#include "AI/NavigationSystemBase.h"

const float AProcNeighborhood::CellSize = 100.f * 20.f;
//const float AProcNeighborhood::CenterLocOffset = GridSize * CellSize / 2;

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
}

bool AProcNeighborhood::RandBool()
{
	return RS.RandRange(0, 1) == 1;
}

void AProcNeighborhood::ProcGen(uint32 randomSeed)
{
	RS = FRandomStream(randomSeed);
	InitializeGrid();
	GenerateRoads();
	InferRoadTypesAndRotations();
	GenerateHouses();
	GenerateMiscellaneousLocations();
	SpawnFinishedNeighborhood();
}

AAProcActor* AProcNeighborhood::SpawnAt(TSubclassOf<AActor> Actor, FVector& Location, FRotator& Rotation) 
{

	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(*FString(Actor->GetName() + "_" + FString::FromInt(ProceduralGenerationIndex)));
	SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Required_Fatal;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.bDeferConstruction = true;

	LastProcActor = GetWorld()->SpawnActor<AAProcActor>(Actor, Location, Rotation, SpawnParams);

	if (LastProcActor)
	{
		LastProcActor->InitializePGI(&ProceduralGenerationIndex);
		LastProcActor->bNetStartup = true;
		LastProcActor->Tags.Add(TEXT("ProcGen"));
		LastProcActor->FinishSpawning(FTransform(Rotation, Location, FVector::OneVector));
	}

	ProceduralGenerationIndex++;

	return LastProcActor;
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
	int32 Lifetime = RS.RandRange(MinLifetime, MaxLifetime);
	Straightness = RS.RandRange(1, 2);
	int32 StartCol = GridSize/2;
	int32 StartRow = GridSize/2;
	UE_LOG(LogTemp, Display, TEXT("Seed ~ Lifetime: %i StartCol: %i StartRow: %i"), Lifetime, StartCol, StartRow);

	EDirection CurrentDirection = EDirection::Down;
	//ChangeDirection(CurrentDirection);
	PlaceRoad(StartCol, StartRow);

	for (int32 i = 0; i < Lifetime; Lifetime--)
	{
		if (RS.RandRange(1, 100) <= BranchingFrequency)
		{
			UE_LOG(LogTemp, Log, TEXT("Branch ~ Lifetime: %i StartCol: %i StartRow: %i"), Lifetime, StartCol, StartRow);
			GenerateRoadBranch(StartCol, StartRow, Lifetime, CurrentDirection);
		}
		if (RS.RandRange(0,Straightness) > 0)
		{
			MoveInDirection(CurrentDirection, StartCol, StartRow);
		}
		else
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
		if (RS.RandRange(1, 100) <= BranchingFrequency)
		{
			UE_LOG(LogTemp, Log, TEXT("Branch ~ Lifetime: %i StartCol: %i StartRow: %i"), Lifetime, StartCol, StartRow);
			GenerateRoadBranch(StartCol, StartRow, Lifetime, CurrentDirection);
		}
		if (RS.RandRange(0, Straightness) > 0)
		{
			MoveInDirection(CurrentDirection, StartCol, StartRow);
		}
		else
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
		if (Row + 1 <= GridSize - 1) ChangeDirection(Direction);
		break;
	case EDirection::Down:
		if (Row - 1 <= 1) ChangeDirection(Direction);
		break;
	case EDirection::Left:
		if (Col - 1 <= 1) ChangeDirection(Direction);
		break;
	case EDirection::Right:
		if (Col + 1 >= GridSize - 1) ChangeDirection(Direction);
		break;
	}

	switch (Direction)
	{
	case EDirection::Up:
		Row++;
		break;
	case EDirection::Down:
		Row--;
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
	EDirection NextDirection;
	do
	{
		TArray<EDirection> Directions = {
		EDirection::Up,
		EDirection::Down,
		EDirection::Left,
		EDirection::Right
		};
		NextDirection = Directions[RS.RandRange(0, Directions.Num() - 1)];
	} while (CurrentDirection == NextDirection);

	CurrentDirection = NextDirection;
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
				if (Row + 1 <= GridSize) //Check up
				{
					if (GridCellTypes[Col][Row + 1] == CellType::Road) ConnectedUp = true;
				}
				if (Row - 1 >= 0) //Check down
				{
					if (GridCellTypes[Col][Row - 1] == CellType::Road) ConnectedDown = true;
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
						GridRotations[Col][Row] = RandBool() ?
								(RandBool() ? CellRotation::Rotation_0DegRight : CellRotation::Rotation_270DegUp):
								(RandBool() ? CellRotation::Rotation_90DegDown : CellRotation::Rotation_180DegLeft);
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
				if (Row + 1 < GridSize) //Check up
				{
					if (GridCellTypes[Col][Row + 1] == CellType::Road) ConnectedUp = true;
				}
				if (Row - 1 > 0) //Check down
				{
					if (GridCellTypes[Col][Row - 1] == CellType::Road) ConnectedDown = true;
				}

				uint8 ConnectionCount = ConnectedRight + ConnectedLeft + ConnectedDown + ConnectedUp;

				ConnectionCounts.Add(FVector2D(Col, Row), ConnectionCount);

				TArray<CellRotation> PossibleRotations;

				if (ConnectedLeft) PossibleRotations.Add(CellRotation::Rotation_90DegDown);
				if (ConnectedRight) PossibleRotations.Add(CellRotation::Rotation_270DegUp);
				if (ConnectedUp) PossibleRotations.Add(CellRotation::Rotation_180DegLeft);
				if (ConnectedDown) PossibleRotations.Add(CellRotation::Rotation_0DegRight);

				if (PossibleRotations.Num() > 0)
				{
					int32 RandomIndex = RS.RandRange(0, PossibleRotations.Num() - 1);
					GridRotations[Col][Row] = PossibleRotations[RandomIndex];
				}
			}
		}
	}

	uint8 HouseCount = RS.RandRange(MinHouseCount, MaxHouseCount);
	for (const auto& Entry : ConnectionCounts)
	{
		if (HouseCount <= 0) continue;
		FVector2D CandidateLocation = Entry.Key;
		uint8 ConnectionCount = Entry.Value;
		int32 Col = CandidateLocation.X;
		int32 Row = CandidateLocation.Y;
		
		float Chance = ConnectionCount * RS.RandRange(0, 50);

		if (Chance > 40)
		{
			GridCellTypes[Col][Row] = CellType::House;
			HouseCount -= 1;
		}
	}
}

void AProcNeighborhood::GenerateMiscellaneousLocations()
{
	TMap<FVector2D, uint8> ConnectionCounts;

	for (int32 Col = 0; Col < GridSize; ++Col)
	{
		for (int32 Row = 0; Row < GridSize; Row++)
		{
			if (MiscThingsToGenerateCount <= 0) return;

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
				if (Row + 1 < GridSize) //Check up
				{
					if (GridCellTypes[Col][Row + 1] == CellType::Road) ConnectedUp = true;
				}
				if (Row - 1 > 0) //Check down
				{
					if (GridCellTypes[Col][Row - 1] == CellType::Road) ConnectedDown = true;
				}
				uint8 ConnectionCount = ConnectedRight + ConnectedLeft + ConnectedDown + ConnectedUp;

				/*
				origin is D3
				6|
				5|
				4|    
				3|        
				2|		  
				1|___________________
				  A  B  C  D  E  F  G
				*/

				if (ConnectionCount == 1)
				{
					if (!bParkGenerated)
					{
						int32 TargetX = Col - ConnectedUp + ConnectedDown;
						int32 TargetY = Row - ConnectedRight + ConnectedLeft;
						if (GridCellTypes[TargetX][TargetY] == CellType::Empty
							&& GridCellTypes[TargetX + 1][TargetY] == CellType::Empty
							&& GridCellTypes[TargetX][TargetY - 1] == CellType::Empty
							&& GridCellTypes[TargetX + 1][TargetY - 1] == CellType::Empty)
						{
							GridCellTypes[TargetX][TargetY] = CellType::Park;
							GridCellTypes[TargetX + 1][TargetY] = CellType::Reserved;
							GridCellTypes[TargetX][TargetY - 1] = CellType::Reserved;
							GridCellTypes[TargetX + 1][TargetY - 1] = CellType::Reserved;
							MiscThingsToGenerateCount--;
							bParkGenerated = true;
						}
						continue;
					}
					if (!bCurvedTowerGenerated)
					{
						TArray<FIntPoint> RelativePositions = {
							FIntPoint(0, 0),
							FIntPoint(-1, 0),
							FIntPoint(-1, -1),
							FIntPoint(0, -1),
							FIntPoint(0, -2),
							FIntPoint(1, -2),
							FIntPoint(1, -1)
						};
						if (CheckAndFill(Col, Row, RelativePositions, CellType::CTower, CellType::Reserved)) {
							--MiscThingsToGenerateCount;
							bCurvedTowerGenerated = true;
						}
						
					}
				}
			}
		}
	}
}

bool AProcNeighborhood::CheckAndFill(int32 Col, int32 Row, const TArray<FIntPoint>& RelativePositions, CellType InitialType, CellType FillType) {
	// Ensure all relative positions are valid and empty
	for (const FIntPoint& Pos : RelativePositions) {
		int32 NewCol = Col + Pos.X;
		int32 NewRow = Row + Pos.Y;

		// Check bounds
		if (NewCol < 0 || NewCol >= GridSize || NewRow < 0 || NewRow >= GridSize) {
			return false;
		}

		// Check if the cell is not empty
		if (GridCellTypes[NewCol][NewRow] != CellType::Empty) {
			return false;
		}
	}

	// All checks passed, set the initial cell and fill the rest
	GridCellTypes[Col][Row] = InitialType; // Set the initial cell
	for (const FIntPoint& Pos : RelativePositions) {
		if (Pos != FIntPoint(0, 0)) { // Skip the initial cell
			int32 NewCol = Col + Pos.X;
			int32 NewRow = Row + Pos.Y;
			GridCellTypes[NewCol][NewRow] = FillType; // Set to reserved or specified fill type
		}
	}
	return true;
}

void AProcNeighborhood::SpawnFinishedNeighborhood()
{
	for (int32 Col = 0; Col < GridSize; ++Col)
	{
		for (int32 Row = 0; Row < GridSize; Row++)
		{
			TSubclassOf<AActor> RoadBlueprint = nullptr;
			float CenterLocOffset = GridSize * CellSize / 2;
			FVector SpawnLocation = GetActorLocation() + FVector(Col * CellSize - CenterLocOffset, -Row * CellSize + CenterLocOffset, 0.0f);
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
						AAProcActor* SpawnedRoad = SpawnAt(RoadBlueprint, SpawnLocation, RoadRotation);
						if (SpawnedRoad)
						{
							SpawnedRoad->RS = FRandomStream(RS.RandRange(0, RAND_MAX));
							SpawnedRoad->ProcGen();
							SpawnedRoads.Add(SpawnedRoad);
						}
					}
				break;
				case CellType::House:
					if (HouseBlueprintClass)
					{
						FRotator HouseRotation = FRotator(0.0f, static_cast<float>(GridRotations[Col][Row]), 0.0f);
						AAProcActor* SpawnedHouse = SpawnAt(HouseBlueprintClass, SpawnLocation, HouseRotation);
						if (SpawnedHouse)
						{
							SpawnedHouse->RS = FRandomStream(RS.RandRange(0,RAND_MAX));
							SpawnedHouse->ProcGen();
							SpawnedHouses.Add(SpawnedHouse);
						}

					}
				break;
				case CellType::Park:
					if (ParkBlueprintClass)
					{
						FRotator ParkRotation = FRotator(0.0f, 0.0f, 0.0f);
						AAProcActor* SpawnedPark = SpawnAt(ParkBlueprintClass, SpawnLocation, ParkRotation);
						if (SpawnedPark)
						{
							SpawnedPark->RS = FRandomStream(RS.RandRange(0, RAND_MAX));
							SpawnedPark->ProcGen();
						}
					}
				break;

				case CellType::CTower:
					if (CTowerBlueprintClass)
					{
						UE_LOG(LogTemp, Error, TEXT("is its spawaned"));
						FRotator CurvedRotation = FRotator(0.0f, 0.0f, 0.0f);
						AAProcActor* SpawnedCurve = SpawnAt(CTowerBlueprintClass, SpawnLocation, CurvedRotation);
						if (SpawnedCurve)
						{
							SpawnedCurve->RS = FRandomStream(RS.RandRange(0, RAND_MAX));
							SpawnedCurve->ProcGen();
							UE_LOG(LogTemp, Error, TEXT("yep its spawaned"));
						}
					}
				break;
				case CellType::Reserved:
					//DrawDebugSphere(GetWorld(),SpawnLocation, 1000.f, 12, FColor::Blue, true);
				break;

				}
			}
		}
	}	
}


