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


	TArray<FVector2D> ConnectedPoints;
	TArray<FVector2D> Midpoints;

	int32 CurrentX = 1;//FMath::RandRange(1, GridSize - 2);
	int32 CurrentY = 1;//FMath::RandRange(1, GridSize - 2);
	FVector2D CurrentPoint = (CurrentX * UnitDistance, CurrentY * UnitDistance);

	int32 Lifetime = FMath::RandRange(1, MaxLifetime);
	EPathDirection CurrentDirection = static_cast<EPathDirection>(FMath::RandRange(0, 3));

	while (Lifetime > 0)
	{
		MoveInDirection(CurrentPoint, CurrentDirection, UnitDistance, ConnectedPoints);
		if (!IsDuplicate(ConnectedPoints, CurrentPoint))
		{
			ConnectedPoints.Add(CurrentPoint);
		}
		--Lifetime;
	}
	GenerateMidpoints(ConnectedPoints,Midpoints);
	SpawnWalls(Midpoints);
}

void AProcHouse::MoveInDirection(FVector2D& Point, EPathDirection& Direction, float Distance, TArray<FVector2D>& ConnectedPoints)
{
	ChangeDirection(Direction);
	switch (Direction)
	{
	case EPathDirection::Up:
		Point.Y = FMath::Clamp(Point.Y + Distance, 0, 1800);
	break;
	case EPathDirection::Down:

		Point.Y = FMath::Clamp(Point.Y - Distance, 0, 1800);
	break;
	case EPathDirection::Left:
		Point.X = FMath::Clamp(Point.X - Distance, 0, 1800);
	break;
	case EPathDirection::Right:
		Point.Y = FMath::Clamp(Point.Y + Distance, 0, 1800);
	break;
	}
}

void AProcHouse::ChangeDirection(EPathDirection& Direction)
{
	uint8 RandomDirection = FMath::RandBool() ? 1 : 3;//FMath::RandRange(1, 3); // 1 is right, 3 is left, 0 is forward
	uint8 iDirection = static_cast<uint8>(Direction);
	//if (RandomDirection == 2) RandomDirection = 0; //dont go backwards ssorry
	Direction = static_cast<EPathDirection>((iDirection + RandomDirection) % 4);
}

void AProcHouse::GenerateMidpoints(const TArray<FVector2D>& ConnectedPoints, TArray<FVector2D>& Midpoints)
{
	Midpoints.Empty();
	UE_LOG(LogTemp, Log, TEXT("midpoint generation started"));

	for (int32 i = 0; i < ConnectedPoints.Num() - 1; i++)
	{
		FVector2D Midpoint;
		Midpoint.X = (ConnectedPoints[i].X + ConnectedPoints[i + 1].X) / 2;
		Midpoint.Y = (ConnectedPoints[i].Y + ConnectedPoints[i + 1].Y) / 2;

		DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(ConnectedPoints[i].X - 900.f, ConnectedPoints[i + 1].Y - 900.f, 0), 120.f, 12, FColor::Red, true);
		DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(Midpoint.X - 900.f, Midpoint.Y - 900.f, 0), 150.f, 12, FColor::Purple, true);

		bool IsWindow = (false);
			/*FMath::IsNearlyEqual(Midpoint.X, 0) ||
			FMath::IsNearlyEqual(Midpoint.Y, 0) ||
			FMath::IsNearlyEqual(Midpoint.Y,GridSize * UnitDistance) || //fix to width/height but not now
			FMath::IsNearlyEqual(Midpoint.X,GridSize * UnitDistance)
		);*/

		if (!IsWindow)
		{
		UE_LOG(LogTemp, Log, TEXT("midpoint y: %f"), Midpoint.X);
		UE_LOG(LogTemp, Log, TEXT("midpoint x: %f"), Midpoint.Y);
			Midpoints.Add(Midpoint);
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


void AProcHouse::SpawnWalls(const TArray<FVector2D>& Midpoints)
{
	for (const FVector2D& SpawnPoint : Midpoints)
	{
		FVector SpawnLocation = GetActorLocation() + FVector(SpawnPoint.X - 900.f, SpawnPoint.Y - 900.f, 0.0f);
		bool IsHorizontal = FMath::RoundToInt(SpawnPoint.Y) % 200 == 0;//600 div 200 but not 300
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
			DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(SpawnPoint.X - 900.f, SpawnPoint.Y - 900.f, 0), 100.f, 12, FColor::Green, true);
			AActor* SpawnedWall = GetWorld()->SpawnActor<AActor>(WallToSpawnBlueprint, SpawnLocation, WallRotation);
		}
	}
}