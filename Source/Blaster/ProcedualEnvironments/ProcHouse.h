// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProcHouse.generated.h"

struct FWallInfo {
	FVector2D Midpoint;
	EWallType WallType;
};

struct FPrefabWallLayout {
	TArray<FWallInfo> WallInfos; // Each entry represents a wall's midpoint and type
};

UENUM(BlueprintType)
enum class EPathDirection : uint8
{
	Right = 0,
	Down = 1,
	Left = 2,
	Up = 3,
};
UENUM(BlueprintType)
enum class ERoomType : uint8
{
	Nothing,
	Hallway,
	Staircase
}; 
UENUM(BlueprintType)
enum class EWallType : uint8
{
	ExtraNothing = 0,
	Nothing = 1,
	Wall = 2,
	Doorway = 3,
	Window = 4,
	NotWindow = 5,
	FrontDoor = 6,
	LockedFrontDoor = 8
};
UENUM(BlueprintType)
enum class EFloorType : uint8
{
	Empty,
	Floor,
	Spikes,
	Water
};

enum class EWallRotation : uint32
{
	Rotation_0DegUp= 0,
	Rotation_90DegRight = 90,
	Rotation_180DegDown = 180,
	Rotation_270DegLeft = 270
};
UCLASS()

class BLASTER_API AProcHouse : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProcHouse();
	/*
		RANDOMIZERS
	*/
	int32 Randomness = FMath::RandRange(0, 20);
	int32 Fear = FMath::RandRange(0, 20);
	int32 Openness = FMath::RandRange(0,20);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:	
	/*exterior*/

	UPROPERTY(EditAnywhere, Category = "Modular Components")
	UStaticMeshComponent* HouseMesh;
	UPROPERTY(EditAnywhere, Category = "Modular Components")
	UStaticMeshComponent* PlotMesh;

	/*floors*/
	UPROPERTY(EditAnywhere, Category = "Floors")
	TSubclassOf<AActor> FloorBlueprint;

	UPROPERTY(EditAnywhere, Category = "Floors")
	TSubclassOf<AActor> SpikesBlueprint;

	UPROPERTY(EditAnywhere, Category = "Floors")
	TSubclassOf<AActor> WaterBlueprint;

	/*walls*/

	UPROPERTY(EditAnywhere, Category = "Prefab Walls")
		TSubclassOf<AActor> WallBlueprint;

	UPROPERTY(EditAnywhere, Category = "Prefab Walls")
		TSubclassOf<AActor> DoorwayBlueprint;

	UPROPERTY(EditAnywhere, Category = "Prefab Walls")
		TSubclassOf<AActor> WindowBlueprint;

	UPROPERTY(EditAnywhere, Category = "Prefab Walls")
		TSubclassOf<AActor> NotWindowBlueprint;

	UPROPERTY(EditAnywhere, Category = "Prefab Walls")
		TSubclassOf<AActor> FrontDoorBlueprint;

	UPROPERTY(EditAnywhere, Category = "Prefab Walls")
		TSubclassOf<AActor> LockedFrontDoorBlueprint;


protected:

	/*
		FLOORS
	*/
	void InitializeFirstFloor();
	void GenerateFloors();
	void SpawnFloors();

	/*
		PROCEDURAL WALLS
	*/
	UFUNCTION()
	void GenerateRooms();
	void MoveInDirection(EPathDirection& Direction, int32& Col, int32& Row);
	void ChangeDirection(EPathDirection& CurrentDirection, int32 Col, int32 Row);
	void InferWallLocations();
	bool IsDuplicate(int32 TargetX, int32 TargetY);
	void DesignateWall(int32 TargetX, int32 TargetY, EWallType WallType);
	void RandomizeWallsAndWindows();
	UFUNCTION()
	void SpawnWalls();

	/*
	*	PREFAB WALLS
	*/
	EWallType ConvertLetterToWallType(const FString& Letter);
	void ReadPrefabLayoutsFromFile();
	void SpawnPrefabWalls();
	void SpawnWall(TSubclassOf<AActor> PFWallBlueprint, const FVector& Location, const FRotator& Rotation);

private:
	TArray<FPrefabWallLayout> PrefabWallLayouts;

	//walls
	static constexpr int32 GridSize = 2;
	UPROPERTY(EditAnywhere, Category = "Walls")
	int32 MaxLifetime = 5;
	const int32 UnitDistance = 600;

	// walls but potentially better
	ERoomType RoomGrid[GridSize][GridSize];
	UPROPERTY()
	TArray<int32> aConnectedWallsX;
	TArray<int32> aConnectedWallsY;
	TArray<EWallType> aWallTypes;

	//narrow walls
	bool GridNWalls[GridSize][GridSize];

	//floors
	static const int32 GridHeight = 3;
	static const int32 GridWidth = 3;
	EFloorType GridFloorTypes[GridWidth][GridHeight];

};
