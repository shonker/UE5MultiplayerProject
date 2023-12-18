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
enum class CellType : uint8
{
	Empty,
	Road
};

UCLASS()
class BLASTER_API AProcNeighborhood : public AActor
{
	GENERATED_BODY()

public:
	AProcNeighborhood();

protected:
	virtual void BeginPlay() override;

	static const int32 GridSize = 8;
	static const float CellSize;

	UPROPERTY(EditDefaultsOnly, Category = "RoadGeneration")
		int32 BranchingFrequency = 10; //percent

	UPROPERTY(EditDefaultsOnly, Category = "RoadGeneration")
		int32 MinLifetime = 5;

	UPROPERTY(EditDefaultsOnly, Category = "Road Generation")
		int32 MaxLifetime = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Road Generation")
		TSubclassOf<AActor> RoadClass;

	CellType Grid[GridSize][GridSize];

	void InitializeGrid();
	void GenerateRoads();
	void GenerateRoadBranch(int32 StartRow, int32 StartCol, int32 Lifetime, EDirection CurrentDirection);

	void MoveInDirection(EDirection Direction, int32& Row, int32& Col);

	void ChangeDirection(EDirection& CurrentDirection);

	void PlaceRoad(int32 Row, int32 Col);

	void SpawnFinishedNeighborhood();

};
