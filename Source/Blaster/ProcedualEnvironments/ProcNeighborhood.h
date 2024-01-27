// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProcNeighborhood.generated.h"

UENUM(BlueprintType)
enum class EDirection : uint8
{
	Up = 0, 
	Down = 1,
	Left = 2,
	Right =3
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
	Road,
	Park,
	Reserved
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
	void ProcGen(uint32 randomSeed);
	class AAProcActor* SpawnAt(TSubclassOf<AActor> Actor, FVector& Location, FRotator& Rotation);
	FRandomStream RS;
	uint32 PGI = 0;

	class AAProcActor* LastProcActor;
protected:
	virtual void BeginPlay() override;
	bool RandBool();
	//virtual void InitGameState() override;

	static const int32 GridSize = 16;
	static const float CellSize;

	/*
		ROADS
	*/

	UPROPERTY(EditDefaultsOnly, Category = "Road Generation")
		TSubclassOf<AActor> ProceduralActor;

	UPROPERTY(EditDefaultsOnly, Category = "Road Generation")
		int32 BranchingFrequency = 4; //percent

	UPROPERTY(EditDefaultsOnly, Category = "Road Generation")
		int32 MinLifetime = 5;

	int32 Straightness; 

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
	/*
		HOUSES
	*/
	UPROPERTY(EditDefaultsOnly, Category = "House Generation")
		TSubclassOf<AActor> HouseBlueprintClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "House Generation")
		int32 MinHouseCount = 5;

	UPROPERTY(EditDefaultsOnly, Category = "House Generation")
		int32 MaxHouseCount = 10;


	CellType GridCellTypes[GridSize][GridSize];
	CellRotation GridRotations[GridSize][GridSize];
	ERoadType GridRoadTypes[GridSize][GridSize];

	void InitializeGrid();
	void GenerateRoads();
	void GenerateRoadBranch(int32 StartRow, int32 StartCol, int32 Lifetime, EDirection CurrentDirection);

	void GenerateHouses();

	void MoveInDirection(EDirection Direction, int32& Row, int32& Col);

	void ChangeDirection(EDirection& CurrentDirection);

	void PlaceRoad(int32 Row, int32 Col);

	void InferRoadTypesAndRotations();

	void SpawnFinishedNeighborhood();
	UPROPERTY(BlueprintReadOnly)
		TArray<AActor*> SpawnedRoads;
	UPROPERTY(BlueprintReadOnly)
		TArray<AActor*> SpawnedHouses;

	/*
		MISC LOCATIONS
	*/
	int32 MiscThingsToGenerateCount = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Park Generation")
		TSubclassOf<AActor> ParkBlueprintClass;

	void GenerateMiscellaneousLocations();
};
