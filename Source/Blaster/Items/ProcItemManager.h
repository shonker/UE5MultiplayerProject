// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Blaster/ProceduralEnvironments/AProcActor.h"
#include "ItemSpawnPoint.h"
#include "Blaster/Weapon/Weapon.h"
#include "ProcItemManager.generated.h"


/**
 * 
 */

USTRUCT(BlueprintType)
struct FSpawnableItems
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AWeapon> ItemClass;
};

USTRUCT(BlueprintType)
struct FItemsToSpawn
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AItemSpawnPoint* SpawnPoint;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AWeapon> ItemToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECurseLevel CurseLevel; //"curse0", "curse1", etc
};

UCLASS()
class BLASTER_API AProcItemManager : public AAProcActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawnable Objects") 
	TArray<FSpawnableItems> SpawnableAlwaysCursed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawnable Objects") 
	TArray<FSpawnableItems> SpawnableSometimesCursed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawnable Objects") 
	TArray<FSpawnableItems> SpawnableWeapons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawnable Objects") 
	TArray<FSpawnableItems> SpawnableMasks;
	
private:

	bool bGunExists = false;
	int32 TotalSpawnedCurseTally = 0;
	int32 CurseTallyNecessaryToWinLevel = 100;
	UPROPERTY()
	TArray<AItemSpawnPoint*> SpawnPoints;
	TArray<FItemsToSpawn> SelectedSpawns;
	
	virtual void ProcGen() override;
	void FindItemSpawns();
	void AssignPredeterminedLocations();
	static int GetPredeterminedPointsForCurseLevel(const ECurseLevel& CurseLevel);
	void CalculateItemDistribution();
	static int GetPointsForCurseLevel(const ECurseLevel& CurseLevel);
	void SpawnItems();
};
