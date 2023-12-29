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
	Nothing = 0,
	Wall = 1,
	Doorway = 2,
	Window = 3,
	NotWindow = 4
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
	EWallType GridWallTypes[GridSize][GridSize][4]; // Added the third dimension for each wall direction
	EWallRotation GridWallRotations[GridSize][GridSize][4]; // Added the third dimension for each wall direction

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
	//void GenerateWalls();
	void GenerateFloors();
	void SpawnFloors();
};
