// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProcNeighborhood.generated.h"

UENUM(BlueprintType)
enum class EDirection : uint8
{
	Up, 
	Down,
	Left,
	Right
};
UENUM(BlueprintType)
enum class ERoadType : uint8
{
	Nothing = 0,
	DeadEnd = 1, 
	TwoWay = 2,
	ThreeWay = 3,
	FourWay = 4,
	TwoWayTurn = 5
};
UENUM(BlueprintType)
enum class CellType : uint8
{
	Empty,
	House,
	Road
};
enum class CellRotation : uint32
{
	Rotation_0DegRight = 0,
	Rotation_90DegDown = 90,
	Rotation_180DegLeft = 180,
	Rotation_270DegUp = 270
};

UCLASS()
class BLASTER_API AProcNeighborhood : public AActor
{
	GENERATED_BODY()

public:
	AProcNeighborhood();

protected:
	virtual void BeginPlay() override;

	static const int32 GridSize = 16;
	static const float CellSize;
	
	UPROPERTY(EditDefaultsOnly, Category = "RoadGeneration")
		int32 BranchingFrequency = 4; //percent

	UPROPERTY(EditDefaultsOnly, Category = "RoadGeneration")
		int32 MinLifetime = 5;

	UPROPERTY(EditDefaultsOnly, Category = "Road Generation")
		int32 MaxLifetime = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Road Generation")
		TSubclassOf<AActor> RoadClass;

	UPROPERTY(EditDefaultsOnly, Category = "Road Generation")
		TSubclassOf<AActor> DeadEndBlueprint;

	UPROPERTY(EditDefaultsOnly, Category = "Road Generation")
		TSubclassOf<AActor> TwoWayBlueprint;

	UPROPERTY(EditDefaultsOnly, Category = "Road Generation")
		TSubclassOf<AActor> ThreeWayBlueprint;

	UPROPERTY(EditDefaultsOnly, Category = "Road Generation")
		TSubclassOf<AActor> FourWayBlueprint;

	UPROPERTY(EditDefaultsOnly, Category = "Road Generation")
		TSubclassOf<AActor> TwoWayTurnBlueprintClass;

	CellType GridCellTypes[GridSize][GridSize];
	CellRotation GridRotations[GridSize][GridSize];
	ERoadType GridRoadTypes[GridSize][GridSize];

	void InitializeGrid();
	void GenerateRoads();
	void GenerateRoadBranch(int32 StartRow, int32 StartCol, int32 Lifetime, EDirection CurrentDirection);

	void MoveInDirection(EDirection Direction, int32& Row, int32& Col);

	void ChangeDirection(EDirection& CurrentDirection);

	void PlaceRoad(int32 Row, int32 Col);

	void InferRoadTypesAndRotations();

	void SpawnFinishedNeighborhood();
	UPROPERTY(BlueprintReadOnly)
		TArray<AActor*> SpawnedRoads;

};
