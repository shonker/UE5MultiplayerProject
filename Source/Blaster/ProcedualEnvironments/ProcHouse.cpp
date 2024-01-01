// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcHouse.h"

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
	if (HasAuthority())
	{
		InitializeFirstFloor();
		GenerateFloors();
		SpawnFloors();
		//GenerateWalls();
		SpawnPrefabWalls();
	}
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
			if (FloorToSpawnBlueprint != nullptr)
			{
				SpawnedFloor = GetWorld()->SpawnActor<AActor>(FloorToSpawnBlueprint, SpawnLocation, FRotator::ZeroRotator);
			}
		}
	}
}


void AProcHouse::GenerateWalls()
{
	//initialize
	for (int32 Col = 0; Col < GridSize; ++Col)
	{
		for (int32 Row = 0; Row < GridSize; Row++)
		{
			WallGrid[Col][Row] = false;
			DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(Col * UnitDistance - 900.f, Row * UnitDistance - 900.f, 0), 75.f, 12, FColor::Blue, true);
		}
	}
	//generate
	int32 Lifetime = FMath::RandRange(1, MaxLifetime);
	int32 StartCol = FMath::RandRange(0, GridSize - 1);
	int32 StartRow = FMath::RandRange(0, GridSize - 1);
	UE_LOG(LogTemp, Display, TEXT("Seed ~ Lifetime: %i StartCol: %i StartRow: %i"), Lifetime, StartCol, StartRow);

	EPathDirection CurrentDirection = static_cast<EPathDirection>(FMath::RandRange(0, 3));

	WallGrid[StartCol][StartRow] = true;

	for (int32 i = 0; i < Lifetime; Lifetime--)
	{
		ChangeDirection(CurrentDirection, StartCol, StartRow);
		MoveInDirection(CurrentDirection, StartCol, StartRow);
	}
	InferWallLocations();
	SpawnWalls();
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

	WallGrid[Col][Row] = true;
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

	CurrentDirection = possibleDirections[FMath::RandRange(0, possibleDirections.size() - 1)];
}

void AProcHouse::InferWallLocations()
{
	for (int32 Col = 0; Col < GridSize; ++Col)
	{
		for (int32 Row = 0; Row < GridSize; Row++)
		{

			if (WallGrid[Col][Row] == true)
			{

				bool ConnectedRight = false;
				bool ConnectedDown = false;
				bool ConnectedLeft = false;
				bool ConnectedUp = false;

				if (Col - 1 >= 0)
				{
					if (WallGrid[Col - 1][Row] == true) ConnectedLeft = true;
				}
				if (Col + 1 < GridSize)
				{
					if (WallGrid[Col + 1][Row] == true) ConnectedRight = true;
				}
				if (Row - 1 >= 0)
				{
					if (WallGrid[Col][Row - 1] == true) ConnectedDown = true;
				}
				if (Row + 1 < GridSize)
				{
					if (WallGrid[Col][Row + 1] == true) ConnectedUp = true;
				}

				int32 TargetX;
				int32 TargetY;
				
				if (ConnectedRight)
				{
					TargetY = Row * UnitDistance;
					TargetX = FMath::RoundToInt32((Col + 0.5) * UnitDistance);
					if (!IsDuplicate(TargetX, TargetY))
					{
						aConnectedWallsX.Add(TargetX);
						aConnectedWallsY.Add(TargetY);
					}
				}				
				if (ConnectedDown)
				{
					TargetY = FMath::RoundToInt32((Row - 0.5) * UnitDistance);
					TargetX =Col * UnitDistance;
					if (!IsDuplicate(TargetX, TargetY))
					{
						aConnectedWallsX.Add(TargetX);
						aConnectedWallsY.Add(TargetY);
					}
				}				
				if (ConnectedLeft)
				{
					TargetY = Row * UnitDistance;
					TargetX = FMath::RoundToInt32((Col - 0.5) * UnitDistance);
					if (!IsDuplicate(TargetX, TargetY))
					{
						aConnectedWallsX.Add(TargetX);
						aConnectedWallsY.Add(TargetY);
					}
				}				
				if (ConnectedUp)
				{
					TargetY = FMath::RoundToInt32((Row + 0.5) * UnitDistance);
					TargetX = Col * UnitDistance;
					if (!IsDuplicate(TargetX, TargetY))
					{
						aConnectedWallsX.Add(TargetX);
						aConnectedWallsY.Add(TargetY);
					}
				}			
			}
		}
	}
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


void AProcHouse::SpawnWalls()
{
	for (int32 i = 0; i < aConnectedWallsX.Num(); i++)
	{
		
		int32 SpawnPointX = aConnectedWallsX[i];
		int32 SpawnPointY = aConnectedWallsY[i];

		UE_LOG(LogTemp, Log, TEXT("wall x: %i y: %i "), SpawnPointX,SpawnPointY);

		FVector SpawnLocation = GetActorLocation() + FVector(SpawnPointX - 900.f, SpawnPointY - 900.f, 0.0f);
		bool IsHorizontal = SpawnPointY % 200 == 0;//600 div 200 but not 300
		float YawRotation = IsHorizontal ? 0.f : 90.f;
		FRotator WallRotation = FRotator(0.0f, YawRotation, 0.0f);

		TSubclassOf<AActor> WallToSpawnBlueprint = nullptr;
		 
		int32 DoorCount = FMath::RandRange(0, 2);
		
		if (DoorCount > 0 && FMath::RandRange(1, 5) == 1)
		{
			WallToSpawnBlueprint = DoorwayBlueprint;
			--DoorCount;
		}
		else
		{
			WallToSpawnBlueprint = WallBlueprint;
		}

		if (WallToSpawnBlueprint != nullptr)
		{
			FColor IsHorizontalColor = IsHorizontal ? FColor::Green : FColor::Red;
			DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(SpawnPointX - 900.f, SpawnPointY - 900.f, 0), 100.f, 12, IsHorizontalColor, true);
			//AActor* SpawnedWall = GetWorld()->SpawnActor<AActor>(WallToSpawnBlueprint, SpawnLocation, WallRotation);
		}
	}
}

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


void AProcHouse::SpawnPrefabWalls()
{
	if (PrefabWallLayouts.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No prefab wall layouts available."));
		return;
	}
	//choose random layout
	int32 LayoutIndex = FMath::RandRange(0, PrefabWallLayouts.Num() - 1);
	FPrefabWallLayout SelectedLayout = PrefabWallLayouts[LayoutIndex];

	for (const FWallInfo& WallInfo : SelectedLayout.WallInfos) {

		FVector SpawnLocation = GetActorLocation() + FVector(WallInfo.Midpoint.X - 900.f, WallInfo.Midpoint.Y - 900.f, 0.0f);

		bool IsHorizontal = FMath::RoundToInt(WallInfo.Midpoint.Y) % 200 == 0;//600 div 200 but not 300
		float YawRotation = IsHorizontal ? 0.f : 90.f;
		FRotator SpawnRotation = FRotator(0.0f, YawRotation, 0.0f);

		switch (WallInfo.WallType) {
		case EWallType::ExtraNothing:
			break;

		case EWallType::Nothing:
			UE_LOG(LogTemp, Log, TEXT("nothing"));

			DrawDebugSphere(GetWorld(), SpawnLocation, 80.f, 12, FColor::Green, true);
			break;

		case EWallType::Wall:
			UE_LOG(LogTemp, Log, TEXT("wall"));
			SpawnWall(WallBlueprint, SpawnLocation, SpawnRotation);
			break;

		case EWallType::Doorway:
			UE_LOG(LogTemp, Log, TEXT("doorway"));

			SpawnWall(DoorwayBlueprint, SpawnLocation, SpawnRotation);
			break;

		case EWallType::Window:
			UE_LOG(LogTemp, Log, TEXT("window"));

			SpawnWall(WindowBlueprint, SpawnLocation, SpawnRotation);
			break;

		case EWallType::NotWindow:
			UE_LOG(LogTemp, Log, TEXT("not window"));

			SpawnWall(NotWindowBlueprint, SpawnLocation, SpawnRotation);
			break;

		case EWallType::FrontDoor:
			UE_LOG(LogTemp, Log, TEXT("front door"));

			SpawnWall(FrontDoorBlueprint, SpawnLocation, SpawnRotation);
			break;

		case EWallType::LockedFrontDoor:
			UE_LOG(LogTemp, Log, TEXT("locked door"));

			SpawnWall(LockedFrontDoorBlueprint, SpawnLocation, SpawnRotation);
			break;

		default:

			break;
		}
	}
}

void AProcHouse::SpawnWall(TSubclassOf<AActor> PFWallBlueprint, const FVector& Location, const FRotator& Rotation)
{
	if (PFWallBlueprint != nullptr)
	{
		DrawDebugSphere(GetWorld(), Location, 100.f, 12, FColor::Purple, true);
		GetWorld()->SpawnActor<AActor>(PFWallBlueprint, Location, Rotation);
	}
}
