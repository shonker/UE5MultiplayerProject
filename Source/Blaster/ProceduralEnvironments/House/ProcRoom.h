// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProcHouse.h"
#include "Blaster/ProceduralEnvironments/AProcActor.h"
#include "ProcRoom.generated.h"

/**
 * 
 */


USTRUCT(BlueprintType)
struct FPlacementTypeObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ObjectClass;

	// Tag to indicate the room location "center floor (cf) center wall (cw) wall floor (wf)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlacementTag;
};

USTRUCT(BlueprintType)
struct FRoomTypeSpawnables
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERoomType RoomType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPlacementTypeObject> SpawnableObjects;
};



UCLASS()
class BLASTER_API AProcRoom : public AAProcActor
{
	GENERATED_BODY()

	
public:
	virtual void ProcGen() override;
	bool AreTransformsEqual(const FTransform& Transform1, const FTransform& Transform2, float Epsilon);
	bool ContainsTransform(const TArray<FTransform>& TransformArray, const FTransform& TransformToCheck, float Epsilon);

	ERoomType RoomType;
	/*
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TArray<TSubclassOf<AActor>> FloorCenterSpawnableObjects;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TArray<TSubclassOf<AActor>> FloorWallSpawnableObjects;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TArray<TSubclassOf<AActor>> MiddleWallSpawnableObjects;
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output Locations")
	TArray<FTransform> FloorCenterOutputTransform;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output Locations")
	TArray<FTransform> FloorWallSpawnableTransforms;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output Locations")
	TArray<FTransform> MiddleWallSpawnableTransforms;

protected:
	//virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	float ObjectExpDecayRate = 0.8f;

	UPROPERTY(EditAnywhere)
	int MaxSpawnedObjects = 6;
	
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TArray<FRoomTypeSpawnables> RoomTypeSpawnables;

	//TArray<FPlacementTypeObject> GetSpawnableObjectsForRoomType(ERoomType RoomType, const FName& PlacementTag);

};
