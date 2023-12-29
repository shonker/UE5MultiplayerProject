// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProcHouse.generated.h"


UENUM(BlueprintType)
enum class EWallDirection : uint8
{
	Up,
	Down,
	Left,
	Right
};
UENUM(BlueprintType)
enum class EWallType : uint8
{
	Initialized = 0,
	Nothing = 1,
	Wall = 2,
	Doorway = 3,
	Window = 4,
	NotWindow = 5
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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	static const int32 GridSize = 2;
	EFloorType GridFloorTypes[GridSize][GridSize];
	
	

	EWallType GridWallTypes[GridSize][GridSize][3]; // Added the third dimension for each wall direction
	EWallRotation GridWallRotations[GridSize][GridSize][3]; // Added the third dimension for each wall direction

	//EWallType HorizWallTypes[GridSize][GridSize + 1]; //[x][0] and [x][num - 1] are bottom and top walls, respectively
	//EWallType VertWallTypes[GridSize + 1][GridSize];  //[0][x] and [num - 1][x] are left and right walls, respectively

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

protected:
	void InitializeFirstFloor();
	
	void GenerateWalls();
	void GenerateFloors();
	void SpawnFloors();
};
