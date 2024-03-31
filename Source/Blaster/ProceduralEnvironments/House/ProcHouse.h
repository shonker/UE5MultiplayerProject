// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blaster/ProceduralEnvironments/AProcActor.h"
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
	Staircase,
	WhiteWood
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
	Water,
	Stairs
};

enum class EWallRotation : uint32
{
	Rotation_0DegUp= 0,
	Rotation_90DegRight = 90,
	Rotation_180DegDown = 180,
	Rotation_270DegLeft = 270
};

UENUM(BlueprintType)
enum class EHouseType : uint8
{
	WhiteWood UMETA(DisplayName="White Wood House Type"),
	BrownWood UMETA(DisplayName="Brown Wood House Type"),
	Spikes UMETA(DisplayName="Spikes House Type"),
	Hito UMETA(DisplayName="Hito House Type"),
	Water UMETA(DisplayName="Water House Type")
};

UCLASS()

class BLASTER_API AProcHouse : public AAProcActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProcHouse();
	virtual void ProcGen() override;
	/*
		RANDOMIZERS
	*/
	int32 Randomness;
	int32 Fear;
	int32 Openness;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:

	EHouseType HouseType;
	
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
	
	UPROPERTY(EditAnywhere, Category = "Floors")
	TSubclassOf<AActor> StaircaseBlueprint;
	
	UPROPERTY(EditAnywhere, Category = "Floors")
	TSubclassOf<AActor> StaircaseMirroredBlueprint;

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

	/*rooms*/

	UPROPERTY(EditAnywhere, Category = "Room Content: White Wood")
	TSubclassOf<AActor> WhiteWoodRoomBlueprint;

	UPROPERTY(EditAnywhere, Category = "Room Content: Brown Wood")
	TSubclassOf<AActor> BrownWoodRoomBlueprint;

	UPROPERTY(EditAnywhere, Category = "Room Content: Spikes")
	TSubclassOf<AActor> SpikeRoomBlueprint;
	
	UPROPERTY(EditAnywhere, Category = "Room Content: Spikes")
	TSubclassOf<AActor> HitoRoomBlueprint;

	UPROPERTY(EditAnywhere, Category = "Room Content: Spikes")
	TSubclassOf<AActor> WaterRoomBlueprint;
	
protected:

	void ChooseHouseType();
	/*
		FLOORS
	*/
	void InitializeFirstFloor();
	void GenerateFloors();
	void SpawnFloors();
	void DivideHouseIntoHallways();
	/*
		ROOMS
	*/
	void AssignRoomTypes();
	
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
