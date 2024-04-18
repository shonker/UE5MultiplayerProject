// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/ProceduralEnvironments/AProcActor.h"
#include "RoadSpawnPoint.h"
#include "RoadSpawnablesManager.generated.h"

USTRUCT(BlueprintType)
struct FRoadObjects
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road Objects")
	TSubclassOf<AAProcActor> RoadObject;
};

USTRUCT(BlueprintType)
struct FSelectedRoadObjects
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ARoadSpawnPoint* SpawnPoint;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AAProcActor> SelectedObject;
};

UCLASS()
class BLASTER_API ARoadSpawnablesManager : public AAProcActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Points") 
	TArray<TSubclassOf<AAProcActor>> CommonRoadObjects;
	
private:

	UPROPERTY()
	TArray<ARoadSpawnPoint*> SpawnPoints;
	TArray<FSelectedRoadObjects> SelectedSpawns;
	
	virtual void ProcGen() override;
	void FindRoadSpawns();
	void CalculateItemDistribution();
	void SpawnRoadObjects();
};
