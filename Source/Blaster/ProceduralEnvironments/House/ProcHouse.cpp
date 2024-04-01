// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcHouse.h"
#include "ProcWall.h"
#include "DrawDebugHelpers.h"
#include "ProcRoom.h"

// Sets default values
AProcHouse::AProcHouse()
{
	ReadPrefabLayoutsFromFile();
	
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
	
	//prefab python proc
	//SpawnPrefabWalls();A

	//generative narrowWalls
	//GenerateNarrowWalls();
}

void AProcHouse::ProcGen()
{
	ChooseHouseType();
	InitializeFirstFloor();
	GenerateFloors();
	SpawnFloorsAndCeilings();

	//shitty self proc
	GenerateRooms();
}

void AProcHouse::ChooseHouseType()
{
	// Initialize arbitrary weights for each house type
	TMap<EHouseType, float> Weights = {
		{EHouseType::WhiteWood, 0.5f},
		//{EHouseType::BrownWood, 0.3f},
		{EHouseType::Spikes, 0.05f},
		{EHouseType::Water, 0.1f},
		//{EHouseType::Hito, 0.05f},
	};

	// Calculate the total weight for normalization
	float TotalWeight = 0.0f;
	for (const auto& Elem : Weights)
	{
		TotalWeight += Elem.Value;
	}

	// Normalize weights and accumulate
	float AccumulatedWeight = 0.0f;
	TMap<EHouseType, float> NormalizedWeights;
	for (const auto& Elem : Weights)
	{
		AccumulatedWeight += Elem.Value / TotalWeight;
		NormalizedWeights.Add(Elem.Key, AccumulatedWeight);
	}

	// Generate a random number in the range [0, 1]
	float RandomNumber = RS.FRand(); // Returns a float between 0.0 and 1.0

	// Determine the house type based on the random number
	for (const auto& Elem : NormalizedWeights)
	{
		if (RandomNumber <= Elem.Value)
		{
			HouseType = Elem.Key;
			return;
		}
	}
	HouseType = Weights.begin()->Key;
}

void AProcHouse::InitializeFirstFloor()
{
	for (int32 Col = 0; Col < GridWidth; ++Col)
	{
		for (int32 Row = 0; Row < GridHeight; ++Row)
		{
			GridFloorTypes[Col][Row] = EFloorType::Empty;
		}
	}
}

void AProcHouse::GenerateFloors()
{
	Randomness = RS.RandRange(0, 20);
	Fear = RS.RandRange(0, 20);
	Openness = RS.RandRange(0, 20);
	int32 RandomValue = RS.RandRange(0, 99);

	
	for (uint8 Col = 0; Col < GridWidth; ++Col)
	{
		for (uint8 Row = 0; Row < GridHeight; ++Row)
		{
			if (HouseType == EHouseType::Water)
			{
				GridFloorTypes[Col][Row] = EFloorType::Water;
				continue;
			}
			if (HouseType == EHouseType::Spikes)
			{
				GridFloorTypes[Col][Row] = EFloorType::Spikes;
				continue;
			}
			else
			{
				//if we are in back corner
				if ((Col == 0 || Col == GridWidth - 1) && Row == 0) 
				{
					if (RS.RandRange(1, 4) == 1)
					{
						GridFloorTypes[Col][Row] = EFloorType::Stairs;
						continue;
					}
					continue;
				}
				GridFloorTypes[Col][Row] = EFloorType::Floor;
				if (RS.RandRange(0, 99) < 10)
				{	
					GridFloorTypes[Col][Row] = EFloorType::Spikes;
				}
			}
		}
	}
}

void AProcHouse::SpawnFloorsAndCeilings()
{
	for (uint8 Col = 0; Col < GridWidth; ++Col)
	{
		for (uint8 Row = 0; Row < GridHeight; ++Row)
		{
			FVector SpawnLocation = GetActorLocation() + FVector(Col * 600.f - 600.f, Row * 600.f - 600.f, 0.0f);
			FRotator SpawnRotation = FRotator::ZeroRotator;
			TSubclassOf<AActor> FloorToSpawnBlueprint = nullptr;

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
			case EFloorType::Stairs:
				FloorToSpawnBlueprint = StaircaseBlueprint;
				break;
			default:
				FloorToSpawnBlueprint = FloorBlueprint;
				break;
			}
			if (FloorToSpawnBlueprint != nullptr)
			{
				AAProcActor* SpawnedFloor = SpawnAt(FloorToSpawnBlueprint, SpawnLocation, SpawnRotation);
				if (GridFloorTypes[Col][Row] != EFloorType::Stairs)
				{
					if (CeilingBlueprint != nullptr)
					AAProcActor* SpawnedCeiling = SpawnAt(CeilingBlueprint, SpawnLocation, SpawnRotation);
				}
			}
		}
	}
}


void AProcHouse::GenerateRooms()
{
	//initialize
	for (int32 Col = 0; Col <= GridSize; Col++)
	{
		for (int32 Row = 0; Row <= GridSize; Row++)
		{
			//UE_LOG(LogTemp, Log, TEXT("row: %i"), Row);
			RoomGrid[Col][Row] = ERoomType::Nothing;
			//DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(Col * UnitDistance - 600, Row * UnitDistance - 600.f, 0), 75.f, 12, FColor::Blue, true);
		}
	}
	DivideHouseIntoRooms();
	//AssignRoomTypes();
	InferWallLocations();
	RandomizeWallsAndWindows();
	SpawnWalls();
}

void AProcHouse::DivideHouseIntoRooms()
{
	//generate
	int32 Lifetime = RS.RandRange(1, MaxLifetime);
	int32 StartCol = RS.RandRange(0, GridSize);
	int32 StartRow = RS.RandRange(0, GridSize);
	//UE_LOG(LogTemp, Display, TEXT("Seed ~ Lifetime: %i StartCol: %i StartRow: %i"), Lifetime, StartCol, StartRow);
	//UE_LOG(LogTemp, Log, TEXT("fear: i%, randomness: i%"),Lifetime, Fear, Randomness);
	EPathDirection CurrentDirection = static_cast<EPathDirection>(RS.RandRange(0, 3));

	RoomGrid[StartCol][StartRow] = ERoomType::Hallway;

	for (int32 i = 0; i < Lifetime; Lifetime--)
	{
		ChangeDirection(CurrentDirection, StartCol, StartRow);
		MoveInDirection(CurrentDirection, StartCol, StartRow);
	}
}
void AProcHouse::MoveInDirection(EPathDirection& Direction, int32& Col, int32& Row)
{
	// Check and adjust direction if at the edge of the grid
	if ((Direction == EPathDirection::Up && Row + 1 >= GridSize) ||
		(Direction == EPathDirection::Down && Row - 1 < 0) ||
		(Direction == EPathDirection::Left && Col - 1 < 0) ||
		(Direction == EPathDirection::Right && Col + 1 >= GridSize)) {
		ChangeDirection(Direction, Col, Row);
	}

	// Move in the direction
	switch (Direction)
	{
	case EPathDirection::Up:
		Row++;
		break;
	case EPathDirection::Down:
		Row--;
		break;
	case EPathDirection::Left:
		Col--;
		break;
	case EPathDirection::Right:
		Col++;
		break;
	}
	Col = FMath::Clamp(Col, 0, GridSize - 1);
	Row = FMath::Clamp(Row, 0, GridSize - 1);

	RoomGrid[Col][Row] = ERoomType::Hallway;
}

void AProcHouse::AssignRoomTypes()
{
	for (int32 Col = 0; Col <= GridSize; Col++)
	{
		for (int32 Row = 0; Row <= GridSize; Row++)
		{
			if (GridFloorTypes[Col][Row] == EFloorType::Stairs ||
				RoomGrid[Col][Row] != ERoomType::Hallway)
			{
				continue;
			}
			switch (HouseType)
			{
			case EHouseType::WhiteWood:
				RoomGrid[Col][Row] = ERoomType::WhiteWood;
				break;
				// case EHouseType::BrownWood:
				//
				// break;
				// case EHouseType::Spikes:
				//
				// break;
				// case EHouseType::Hito:
				//
				// break;
			default:

				break;
			}
		}
	}
}
//unimplemented because terribly convoluted and broken lol
void AProcHouse::SpawnRooms()
{
	for (uint8 Col = 0; Col < GridWidth; ++Col)
	{
		for (uint8 Row = 0; Row < GridHeight; ++Row)
		{
			if (RoomGrid[Col][Row] == ERoomType::Nothing) return;

			FVector SpawnLocation = GetActorLocation() + FVector(Col * 600.f - 600.f, Row * 600.f - 600.f, 0.0f);
			FRotator SpawnRotation = FRotator::ZeroRotator;
			
			if (RoomBlueprint != nullptr)
			{
				AProcRoom* SpawnedRoom = Cast<AProcRoom>(SpawnAt(RoomBlueprint, SpawnLocation, SpawnRotation));
				SpawnedRoom->RoomType = RoomGrid[Col][Row];
			}
		}
	}
}

void AProcHouse::ChangeDirection(EPathDirection& CurrentDirection, int32 Col, int32 Row)
{
	std::vector<EPathDirection> possibleDirections = { EPathDirection::Up, EPathDirection::Down, EPathDirection::Left, EPathDirection::Right };

	possibleDirections.erase(std::remove_if(possibleDirections.begin(), possibleDirections.end(),
		[Col, Row, this](EPathDirection dir) {
			return (dir == EPathDirection::Up && Row + 1 >= GridSize) ||
				(dir == EPathDirection::Down && Row - 1 < 0) ||
				(dir == EPathDirection::Left && Col - 1 < 0) ||
				(dir == EPathDirection::Right && Col + 1 >= GridSize);
		}),
		possibleDirections.end());

	CurrentDirection = possibleDirections[RS.RandRange(0, possibleDirections.size() - 1)];
}

void AProcHouse::InferWallLocations()
{
	for (int32 Col = 0; Col <= GridSize; ++Col)
	{
		for (int32 Row = 0; Row <= GridSize; Row++)
		{

			bool WallRight = false;
			bool WallDown = false;
			bool WallLeft = false;
			bool WallUp = false;

			bool WindowLeft = false;
			bool WindowDown = false;
			bool WindowRight = false;
			bool WindowUp = false;

			if (Col - 1 >= 0)
			{
				if (RoomGrid[Col - 1][Row] == ERoomType::Nothing)
				{
					if (RoomGrid[Col][Row] != ERoomType::Nothing)
					{
						WallLeft = true;
					}
					else
					{
						if (Randomness > RS.RandRange(0, 100))
						{
							WallLeft = true;
						}
					}
				}
			}
			else
			{
				WindowLeft = true;
			}
			if (Col + 1 <= GridSize)
			{
				if (RoomGrid[Col + 1][Row] == ERoomType::Nothing)
				{
					if (RoomGrid[Col][Row] != ERoomType::Nothing)
					{
						WallRight = true;
					}
					else
					{
						if (Randomness > RS.RandRange(0, 100))
						{
							WallRight = true;
						}
					}
				}
			}
			else
			{
				WindowRight = true;
			}
			if (Row - 1 >= 0)
			{
				if (RoomGrid[Col][Row - 1] == ERoomType::Nothing)
				{
					if (RoomGrid[Col][Row] != ERoomType::Nothing)
					{
						WallDown = true;
					}
					else
					{
						if (Randomness > RS.RandRange(0, 100))
						{
							WallDown = true;
						}
					}
				}
			}
			else
			{
				WindowDown = true;
			}
			if (Row + 1 <= GridSize)
			{
				if (RoomGrid[Col][Row + 1] == ERoomType::Nothing)
				{
					if (RoomGrid[Col][Row] != ERoomType::Nothing)
					{
						WallUp = true;
					}
					else
					{
						if (Randomness > RS.RandRange(0, 100))
						{
							WallUp = true;
						}
					}
				}
			}
			else 
			{
				WindowUp = true;
			}

			if (Fear > RS.RandRange(0, 100))
			{
				if (Col + 1 <= GridSize) WallRight = true;
				if (Row - 1 >= 0) WallDown = true;
				if (Col - 1 >= 0) WallLeft = true;
				if (Row + 1 <= GridSize) WallUp = true;
			}

			/*if (Openness > RS.RandRange(0, 200))
			{
				 WallRight = false;
				 WallDown = false;
				 WallLeft = false;
				 WallUp = false;
			}*/
				
			int32 TargetX;
			int32 TargetY;
				
			if (WallRight || WindowRight)
			{
				TargetY = Row * UnitDistance;
				TargetX = FMath::RoundToInt32((Col + 0.5) * UnitDistance);
				if (!IsDuplicate(TargetX, TargetY))
				{
					if (WallRight)
					{
						DesignateWall(TargetX, TargetY, EWallType::Wall);
					}
					if (WindowRight)
					{
						DesignateWall(TargetX, TargetY, EWallType::Window);
					}
						
				}
			}	
			if (WallDown || WindowDown)
			{
				TargetY = FMath::RoundToInt32((Row - 0.5) * UnitDistance);
				TargetX = Col * UnitDistance;
				if (!IsDuplicate(TargetX, TargetY))
				{
					if (WallDown)
					{
						DesignateWall(TargetX, TargetY, EWallType::Wall);
					}
					if (WindowDown)
					{
							DesignateWall(TargetX, TargetY, EWallType::Window);
					}
				}
			}				
			if (WallLeft || WindowLeft)
			{
				TargetY = Row * UnitDistance;
				TargetX = FMath::RoundToInt32((Col - 0.5) * UnitDistance);
				if (!IsDuplicate(TargetX, TargetY))
				{
					if (WallLeft)
					{
						DesignateWall(TargetX, TargetY, EWallType::Wall);
					}
					if (WindowLeft)
					{
						DesignateWall(TargetX, TargetY, EWallType::Window);
					}
				}
			}				
			if (WallUp || WindowUp)
			{
				TargetY = FMath::RoundToInt32((Row + 0.5) * UnitDistance);
				TargetX = Col * UnitDistance;
				if (!IsDuplicate(TargetX, TargetY))
				{
					if (WallUp)
					{
						DesignateWall(TargetX, TargetY, EWallType::Wall);
					}
					if (WindowUp)
					{
						if (Col == 1)
						{
							DesignateWall(TargetX, TargetY, EWallType::FrontDoor);
						}
						else
						{
							DesignateWall(TargetX, TargetY, EWallType::Window);
						}
					}
				}
			}	
		}
	}
}

void AProcHouse::DesignateWall(int32 TargetX, int32 TargetY, EWallType WallType)
{
	aConnectedWallsX.Add(TargetX);
	aConnectedWallsY.Add(TargetY);
	aWallTypes.Add(WallType);
}

bool AProcHouse::IsDuplicate(int32 TargetX, int32 TargetY)
{
	for (int32 u = 0; u < aConnectedWallsX.Num(); u++)
	{
		if (aConnectedWallsX[u] == TargetX &&
			aConnectedWallsY[u] == TargetY)
		{
			return true;
		}
	}
	return false;
}

void AProcHouse::RandomizeWallsAndWindows()
{
	for (EWallType& Wall : aWallTypes)
	{
		switch (Wall)
		{
		case EWallType::Window:
			if (Fear > RS.RandRange(0, 100))
			{
				if (RS.RandRange(1, 6) > 1)
				{
					Wall = EWallType::NotWindow;
				}
			} 
			else
			{
				if (RS.RandRange(1, 3) > 1)
				{
					Wall = EWallType::NotWindow;
				}
			}
			break;
		case EWallType::Wall:
			if (Openness > RS.RandRange(0, 100))
			{
				if (RS.RandRange(1, 3) == 1)
				{
					Wall = EWallType::Doorway;
				}
			}
			else
			{
				if (RS.RandRange(1, 6) == 1)
				{
					Wall = EWallType::Doorway;
				}
			}
			break;
		}
	}
}
void AProcHouse::SpawnWalls()
{
	int32 HouseOffsetXY = 600;
	FTransform OwnerTransform = GetActorTransform();

	for (int32 i = 0; i < aConnectedWallsX.Num(); i++)
	{
		int32 SpawnPointX = aConnectedWallsX[i];
		int32 SpawnPointY = aConnectedWallsY[i];

		FVector RelativeSpawnLocation = FVector(SpawnPointX - HouseOffsetXY, SpawnPointY - HouseOffsetXY, 0.0f);
		FVector SpawnLocation = OwnerTransform.TransformPosition(RelativeSpawnLocation); // Apply the owner's transform

		bool IsHorizontal = SpawnPointY % 200 == 0; //600 div 200 but not 300
		float YawRotation = IsHorizontal ? 90.f : 0.f;
		FRotator WallRotation = FRotator(0.0f, YawRotation, 0.0f);
		WallRotation = OwnerTransform.TransformRotation(WallRotation.Quaternion()).Rotator(); // Apply the owner's rotation

		TSubclassOf<AActor> WallToSpawnBlueprint = nullptr;
		switch (aWallTypes[i])
		{
		case EWallType::Wall:
			WallToSpawnBlueprint = WallBlueprint;
			break;
			
		case EWallType::Window:
			WallToSpawnBlueprint = WindowBlueprint;
			break;		
		case EWallType::Doorway:
			WallToSpawnBlueprint = DoorwayBlueprint;
			break;		
		case EWallType::NotWindow:
			WallToSpawnBlueprint = NotWindowBlueprint;
			break;		
		case EWallType::FrontDoor:
			WallToSpawnBlueprint = FrontDoorBlueprint;
			break;		
		}

		if (WallToSpawnBlueprint != nullptr)
		{
			AAProcActor* SpawnedWall = SpawnAt(WallToSpawnBlueprint, SpawnLocation, WallRotation);
			SpawnedWall->ProcGen();
		}
	}
}

//initial prefab concept
void AProcHouse::ReadPrefabLayoutsFromFile()
{
	FString FilePath = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("PrefabRooms.csv"));
	TArray<FString> Lines;
	FFileHelper::LoadFileToStringArray(Lines, *FilePath);

	int32 CurrentHouseStartLine = 0;
	while (CurrentHouseStartLine < Lines.Num())
	{
		FPrefabWallLayout PrefabLayout;
		for (int32 i = 0; i < 7; ++i) { // Each house has 7 rows
			int32 LineIndex = CurrentHouseStartLine + i;
			if (LineIndex < Lines.Num()) {
				FString& Line = Lines[LineIndex];
				TArray<FString> Tokens;
				Line.ParseIntoArray(Tokens, TEXT(","), true);

				for (int32 j = 0; j < Tokens.Num(); ++j) {
//UE_LOG(LogTemp, Log, TEXT("%s"),*Tokens[j]);
					EWallType WallType = ConvertLetterToWallType(Tokens[j]);
					FVector2D WallMidpoint(j * 300, i * 300);

					FWallInfo WallInfo;
					WallInfo.Midpoint = WallMidpoint;
					WallInfo.WallType = WallType;
					PrefabLayout.WallInfos.Add(WallInfo);
				}
			}
		}
		PrefabWallLayouts.Add(PrefabLayout);
		CurrentHouseStartLine += 7;
	}
}

//initial prefab concept
EWallType AProcHouse::ConvertLetterToWallType(const FString& Letter)
{
	if (Letter == "W") return EWallType::Wall;
	if (Letter == "D") return EWallType::Doorway;
	if (Letter == "I") return EWallType::Window;
	if (Letter == "N") return EWallType::NotWindow;
	if (Letter == "U") return EWallType::FrontDoor;
	if (Letter == "L") return EWallType::LockedFrontDoor;
	return EWallType::Nothing; // Default
}

//initial prefab concept
void AProcHouse::SpawnPrefabWalls()
{
	if (PrefabWallLayouts.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No prefab wall layouts available."));
		return;
	}
	//choose random layout
	int32 LayoutIndex = RS.RandRange(0, PrefabWallLayouts.Num() - 1);
	FPrefabWallLayout SelectedLayout = PrefabWallLayouts[LayoutIndex];

	for (const FWallInfo& WallInfo : SelectedLayout.WallInfos) {

		FVector SpawnLocation = GetActorLocation() + FVector(WallInfo.Midpoint.X - 900.f, WallInfo.Midpoint.Y - 900.f, 0.0f);

		bool IsHorizontal = FMath::RoundToInt(WallInfo.Midpoint.Y) % 200 == 0;//600 div 200 but not 300
		float YawRotation = IsHorizontal ? 0.f : 90.f;
		FRotator SpawnRotation = FRotator(0.0f, YawRotation, 0.0f);

		TSubclassOf<AActor> WallToSpawn;

		switch (WallInfo.WallType) {
		case EWallType::ExtraNothing:
			break;
		case EWallType::Nothing:
			break;
		case EWallType::Wall:
			WallToSpawn = WallBlueprint;
			break;
		case EWallType::Doorway:
			WallToSpawn = DoorwayBlueprint;
			break;
		case EWallType::Window:
			WallToSpawn = WindowBlueprint;
			break;
		case EWallType::NotWindow:
			WallToSpawn = NotWindowBlueprint;
			break;
		case EWallType::FrontDoor:
			WallToSpawn = FrontDoorBlueprint;
			break;
		case EWallType::LockedFrontDoor:
			WallToSpawn = LockedFrontDoorBlueprint;
			break;
		default:
		break;
		}

		SpawnAt(WallToSpawn, SpawnLocation, SpawnRotation);

		/*if (WallToSpawn == FrontDoorBlueprint)
		{
			if (HasAuthority()) GetWorld()->SpawnActor<AActor>(WallToSpawn, SpawnLocation, SpawnRotation);
		}
		else
		{
			SpawnAt(WallToSpawn, SpawnLocation, SpawnRotation);
		}*/
		
	}
}
