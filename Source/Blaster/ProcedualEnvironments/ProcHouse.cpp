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
		GenerateWalls();
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
		/*if (FMath::RandRange(1, 100) <= BranchingFrequency)
		{
			UE_LOG(LogTemp, Log, TEXT("Branch ~ Lifetime: %i StartCol: %i StartRow: %i"), Lifetime, StartCol, StartRow);
			GenerateRoadBranch(StartCol, StartRow, Lifetime, CurrentDirection);
		}*/
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
	InferWallLocations();
	SpawnWalls();
}



void AProcHouse::MoveInDirection(EPathDirection Direction, int32& Col, int32& Row)
{

	//check if walking into an unsafe border area
	switch (Direction)
	{
	case EPathDirection::Up:
		if (Row + 1 >= GridSize) ChangeDirection(Direction);
		break;
	case EPathDirection::Down:
		if (Row + 1 < 0) ChangeDirection(Direction);
		break;
	case EPathDirection::Left:
		if (Col - 1 < 0) ChangeDirection(Direction);
		break;
	case EPathDirection::Right:
		if (Col + 1 >= GridSize) ChangeDirection(Direction);
		break;
	}

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

	Col = FMath::Clamp(Col, 0, GridSize-1);
	Row = FMath::Clamp(Row, 0, GridSize-1);

	WallGrid[Col][Row] = true;
}


void AProcHouse::ChangeDirection(EPathDirection& CurrentDirection)
{
	int32 RawDirection = static_cast<int32>(CurrentDirection);
	int32 LeftOrRight = FMath::RandBool() ? -1 : 1;
	int32 NextRawDirection = (RawDirection + LeftOrRight + 3) % 3;
	NextRawDirection = FMath::Clamp(NextRawDirection, 0, 3);//jus2bsafe
	//OutParameter
	CurrentDirection = static_cast<EPathDirection>(NextRawDirection);
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
					aConnectedWallsX.Add(TargetX);
					aConnectedWallsY.Add(TargetY);
				}				
				if (ConnectedDown)
				{
					TargetY = FMath::RoundToInt32((Row - 0.5) * UnitDistance);
					TargetX =Col * UnitDistance;
					aConnectedWallsX.Add(TargetX);
					aConnectedWallsY.Add(TargetY);
				}				
				if (ConnectedLeft)
				{
					TargetY = Row * UnitDistance;
					TargetX = FMath::RoundToInt32((Col - 0.5) * UnitDistance);
					aConnectedWallsX.Add(TargetX);
					aConnectedWallsY.Add(TargetY);
				}				
				if (ConnectedUp)
				{
					TargetY = FMath::RoundToInt32((Row + 0.5) * UnitDistance);
					TargetX = Col * UnitDistance;
					aConnectedWallsX.Add(TargetX);
					aConnectedWallsY.Add(TargetY);
				}			}
		}
	}
}

bool AProcHouse::IsDuplicate(const TArray<FVector2D>& Array, const FVector2D& Point)
{
	for (const FVector2D& ExistingPoint : Array)
	{
		if (FMath::IsNearlyEqual(ExistingPoint.X,Point.X,1.f) 
			&& FMath::IsNearlyEqual(ExistingPoint.Y,Point.Y,1.f))
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