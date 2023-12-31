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
		}
	}
	//generate
	int32 Lifetime = FMath::RandRange(1, MaxLifetime);
	int32 StartCol = GridSize / 2;
	int32 StartRow = GridSize / 2;
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
	/// <summary>
	/// //
	/// </summary>
	//TArray<FVector2D> ConnectedPoints;
	//TArray<FVector2D> Midpoints;

	//int32 CurrentX = 1;//FMath::RandRange(1, GridSize - 2);
	//int32 CurrentY = 1;//FMath::RandRange(1, GridSize - 2);
	//FVector2D CurrentPoint = (CurrentX * UnitDistance, CurrentY * UnitDistance);

	//int32 Lifetime = FMath::RandRange(1, MaxLifetime);
	//EPathDirection CurrentDirection = static_cast<EPathDirection>(FMath::RandRange(0, 3));

	//while (Lifetime > 0)
	//{
	//	MoveInDirection(CurrentPoint, CurrentDirection, UnitDistance, ConnectedPoints);
	//	if (!IsDuplicate(ConnectedPoints, CurrentPoint))
	//	{
	//		ConnectedPoints.Add(CurrentPoint);
	//	}
	//	--Lifetime;
	//}
	//GenerateMidpoints(ConnectedPoints,Midpoints);
	//SpawnWalls(Midpoints);
}



void AProcHouse::MoveInDirection(EPathDirection Direction, int32& Col, int32& Row)
{

	//check if walking into an unsafe border area
	switch (Direction)
	{
	case EPathDirection::Up:
		if (Row - 1 <= 1) ChangeDirection(Direction);
		break;
	case EPathDirection::Down:
		if (Row + 1 >= GridSize - 2) ChangeDirection(Direction);
		break;
	case EPathDirection::Left:
		if (Col - 1 <= 1) ChangeDirection(Direction);
		break;
	case EPathDirection::Right:
		if (Col + 1 >= GridSize - 2) ChangeDirection(Direction);
		break;
	}

	switch (Direction)
	{
	case EPathDirection::Up:
		Row--;
		break;
	case EPathDirection::Down:
		Row++;
		break;
	case EPathDirection::Left:
		Col--;
		break;
	case EPathDirection::Right:
		Col++;
		break;
	}

	Col = FMath::Clamp(Col, 1, GridSize - 2);
	Row = FMath::Clamp(Row, 1, GridSize - 2);

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

				if (Col - 1 >= 0) //Check left
				{
					if (WallGrid[Col - 1][Row] == true) ConnectedLeft = true;
				}
				if (Col + 1 < GridSize) //Check right
				{
					if (WallGrid[Col + 1][Row] == true) ConnectedRight = true;
				}
				if (Row - 1 >= 0) //Check up
				{
					if (WallGrid[Col][Row - 1] == true) ConnectedUp = true;
				}
				if (Row + 1 < GridSize) //Check down
				{
					if (WallGrid[Col][Row + 1] == true) ConnectedDown = true;
				}
				
				if (ConnectedRight)
				{
					int32 TargetCol = Col + UnitDistance / 2;
					if (TargetCol >= 0 && TargetCol <= GridSize)
					{
						// Add the target element to the array
						ConnectedWallsArray.Add(WallGrid[TargetCol][Row]);
					}

				}
			}
		}
	}
}
//
//void AProcHouse::MoveInDirection(FVector2D& Point, EPathDirection& Direction, float Distance, TArray<FVector2D>& ConnectedPoints)
//{
//	ChangeDirection(Direction);
//	switch (Direction)
//	{
//	case EPathDirection::Up:
//		Point.Y = FMath::Clamp(Point.Y + Distance, 0, 1800);
//	break;
//	case EPathDirection::Down:
//
//		Point.Y = FMath::Clamp(Point.Y - Distance, 0, 1800);
//	break;
//	case EPathDirection::Left:
//		Point.X = FMath::Clamp(Point.X - Distance, 0, 1800);
//	break;
//	case EPathDirection::Right:
//		Point.Y = FMath::Clamp(Point.Y + Distance, 0, 1800);
//	break;
//	}
//}
//
//void AProcHouse::ChangeDirection(EPathDirection& Direction)
//{
//	uint8 RandomDirection = FMath::RandBool() ? 1 : 3;//FMath::RandRange(1, 3); // 1 is right, 3 is left, 0 is forward
//	uint8 iDirection = static_cast<uint8>(Direction);
//	//if (RandomDirection == 2) RandomDirection = 0; //dont go backwards ssorry
//	Direction = static_cast<EPathDirection>((iDirection + RandomDirection) % 4);
//}


//
//void AProcHouse::GenerateMidpoints(const TArray<FVector2D>& ConnectedPoints, TArray<FVector2D>& Midpoints)
//{
//	Midpoints.Empty();
//	UE_LOG(LogTemp, Log, TEXT("midpoint generation started"));
//
//	for (int32 i = 0; i < ConnectedPoints.Num() - 1; i++)
//	{
//		FVector2D Midpoint;
//		Midpoint.X = (ConnectedPoints[i].X + ConnectedPoints[i + 1].X) / 2;
//		Midpoint.Y = (ConnectedPoints[i].Y + ConnectedPoints[i + 1].Y) / 2;
//
//		DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(ConnectedPoints[i].X - 900.f, ConnectedPoints[i + 1].Y - 900.f, 0), 120.f, 12, FColor::Red, true);
//		DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(Midpoint.X - 900.f, Midpoint.Y - 900.f, 0), 150.f, 12, FColor::Purple, true);
//
//		bool IsWindow = (false);
//			/*FMath::IsNearlyEqual(Midpoint.X, 0) ||
//			FMath::IsNearlyEqual(Midpoint.Y, 0) ||
//			FMath::IsNearlyEqual(Midpoint.Y,GridSize * UnitDistance) || //fix to width/height but not now
//			FMath::IsNearlyEqual(Midpoint.X,GridSize * UnitDistance)
//		);*/
//
//		if (!IsWindow)
//		{
//		UE_LOG(LogTemp, Log, TEXT("midpoint y: %f"), Midpoint.X);
//		UE_LOG(LogTemp, Log, TEXT("midpoint x: %f"), Midpoint.Y);
//			Midpoints.Add(Midpoint);
//		}
//	}
//}

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