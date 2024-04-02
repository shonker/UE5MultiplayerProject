// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/ProceduralEnvironments/AProcActor.h"
#include "ProcFurniture.generated.h"

/**
 * 
 */


USTRUCT(BlueprintType)
struct FFurnitureSpawnables
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AProcFurniture> ObjectClass;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Tag;  // "clock", "scary", etc
};

UCLASS()
class BLASTER_API AProcFurniture : public AAProcActor
{
	GENERATED_BODY()

public:
	virtual void ProcGen() override;
	void GenerateFurniture();
	
	UPROPERTY(EditAnywhere)
	bool bGeneratesMoreFurniture = false;

	UPROPERTY(EditAnywhere)
	bool bRandomRotateYawAndRollOfGeneratedFurniture = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawnable Objects")
	TArray<FFurnitureSpawnables> SpawnableObjects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FTransform> SpawnableObjectTransforms;

	int32 CalculateObjectsToSpawn(int32 NumTransforms) const;

};
 