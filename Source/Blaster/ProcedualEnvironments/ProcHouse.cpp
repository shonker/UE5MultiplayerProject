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
			/*
			i know the math here is a little confusing but it makes sense
			we are starting from the center of the house. The house is 1800 X 1800
			the house is separated into 3x3 units (but zero indexed)
			we start in the center of the house, at 900,900
			to get to the bottom left corner of the house we need to get to 0,0 so -900 and -900 those values
			but to get to the center of the unit at 0,0 ... which is essentially 0.5,0.5, we need to move +300, +300
			which sums to -600
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
			if (FloorToSpawnBlueprint != nullptr)
			{
				SpawnedFloor = GetWorld()->SpawnActor<AActor>(FloorToSpawnBlueprint, SpawnLocation, FRotator::ZeroRotator);
			}
		}
	}
}


void AProcHouse::GenerateWalls()
{
	TArray<FVector2D> ConnectedPoints;
	TArray<FVector2D> Midpoints;
	for (int32 i = 0; i < GridSize; ++i)
	{
		for (int32 j = 0; j < GridSize; ++j)
		{
			Grid[i][j] = FVector2D(i * UnitDistance, j * UnitDistance);
		}
	}

	int32 CurrentX = FMath::RandRange(0, GridSize - 1);
	int32 CurrentY = FMath::RandRange(0, GridSize - 1);
	FVector2D CurrentPoint = Grid[CurrentX][CurrentY];

	int32 Lifetime = FMath::RandRange(1, MaxLifetime);
	EPathDirection CurrentDirection = static_cast<EPathDirection>(FMath::RandRange(0, 3));

	while (Lifetime > 0)
	{
		MoveInDirection(CurrentPoint, CurrentDirection, UnitDistance);
		ConnectedPoints.Add(CurrentPoint);
		--Lifetime;
	}
	GenerateMidpoints(ConnectedPoints,Midpoints);
	SpawnWalls(Midpoints);
}

void AProcHouse::MoveInDirection(FVector2D& Point, EPathDirection& Direction, float Distance)
{
	ChangeDirection(Direction);
	switch (Direction)
	{
	case EPathDirection::Up:
		if (Point.Y + Distance > 1800)
		{
			MoveInDirection(Point, Direction, Distance);
			break;
		}
		Point.Y += Distance;
		break;
	case EPathDirection::Down:
		if (Point.Y - Distance < 0)
		{
			MoveInDirection(Point, Direction, Distance);
			break;
		}
		Point.Y -= Distance;
		break;
	case EPathDirection::Left:
		if (Point.X + Distance > 1800)
		{
			MoveInDirection(Point, Direction, Distance);
			break;
		}
		Point.X -= Distance;
		break;
	case EPathDirection::Right:
		if (Point.X - Distance < 0)
		{
			MoveInDirection(Point, Direction, Distance);
			break;
		}
		Point.X += Distance;
		break;
	}
}

void AProcHouse::ChangeDirection(EPathDirection& Direction)
{
	uint8 RandomDirection = FMath::RandRange(1, 3); // 1 is right, 3 is left, 0 is forward
	uint8 iDirection = static_cast<uint8>(Direction);
	if (RandomDirection == 2) RandomDirection = 0; //dont go backwards ssorry
	Direction = static_cast<EPathDirection>((iDirection + RandomDirection) % 4);
}

void AProcHouse::GenerateMidpoints(const TArray<FVector2D>& ConnectedPoints, TArray<FVector2D>& Midpoints)
{
	Midpoints.Empty();

	for (int32 i = 0; i < ConnectedPoints.Num() - 1; i++)
	{
		FVector2D Midpoint;
		Midpoint.X = (ConnectedPoints[i].X + ConnectedPoints[i + 1].X) / 2;
		Midpoint.Y = (ConnectedPoints[i].Y + ConnectedPoints[i + 1].Y) / 2;

		bool IsWindow = (
			FMath::IsNearlyEqual(Midpoint.X,0) ||
			FMath::IsNearlyEqual(Midpoint.Y, 0) ||
			FMath::IsNearlyEqual(Midpoint.Y,GridSize * UnitDistance) || //fix to width/height but not now
			FMath::IsNearlyEqual(Midpoint.X,GridSize * UnitDistance)
		);

		if (!IsWindow && !IsDuplicate(Midpoints, Midpoint))
		{
			Midpoints.Add(Midpoint);
		}
	}
}

bool AProcHouse::IsDuplicate(const TArray<FVector2D>& Array, const FVector2D& Point)
{
	for (const FVector2D& ExistingPoint : Array)
	{
		if (FMath::IsNearlyEqual(ExistingPoint.X,Point.X) 
			&& FMath::IsNearlyEqual(ExistingPoint.Y,Point.Y))
		{
			return true;
		}
	}
	return false;
}


void AProcHouse::SpawnWalls(const TArray<FVector2D>& Midpoints)
{
	for (const FVector2D& SpawnPoint : Midpoints)
	{
		FVector SpawnLocation = GetActorLocation() + FVector(SpawnPoint.X, SpawnPoint.Y, 0.0f);
		bool IsHorizontal = FMath::RoundToInt(SpawnPoint.Y) % 100 == 0;
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

			AActor* SpawnedRoad = GetWorld()->SpawnActor<AActor>(WallToSpawnBlueprint, SpawnLocation, WallRotation);
		}
	}
}