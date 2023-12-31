// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProcHouse.generated.h"


UENUM(BlueprintType)
enum class EPathDirection : uint8
{
	Up = 0,
	Right = 1,
	Down = 2,
	Left = 3
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

	//walls
	static constexpr int32 GridSize = 3;
	FVector2D Grid[GridSize][GridSize];
	UPROPERTY(EditAnywhere, Category = "Walls")
	int32 MaxLifetime = 5;
	const int32 UnitDistance = 600;

	//floors
	static const int32 GridHeight = 3;
	static const int32 GridWidth = 3;
	EFloorType GridFloorTypes[GridWidth][GridHeight];

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

	UPROPERTY(EditAnywhere, Category = "Walls")
		TSubclassOf<AActor> WallBlueprint;

	UPROPERTY(EditAnywhere, Category = "Walls")
		TSubclassOf<AActor> WindowBlueprint;

	UPROPERTY(EditAnywhere, Category = "Walls")
		TSubclassOf<AActor> DoorwayBlueprint;

protected:
	void InitializeFirstFloor();
	
	void GenerateFloors();
	void SpawnFloors();


	UFUNCTION()
	void GenerateWalls();
	void MoveInDirection(FVector2D& Point, EPathDirection& Direction, float Distance, TArray<FVector2D>& ConnectedPoints);
	void ChangeDirection(EPathDirection& Direction);
	void GenerateMidpoints(const TArray<FVector2D>& ConnectedPoints, TArray<FVector2D>& Midpoints);
	bool IsDuplicate(const TArray<FVector2D>& Array, const FVector2D& Point);
	void SpawnWalls(const TArray<FVector2D>& Midpoints);


};
