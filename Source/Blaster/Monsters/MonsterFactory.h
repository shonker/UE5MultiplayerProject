// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MonsterSpawnPoint.h"
#include "Blaster/ProceduralEnvironments/Roads/RoadSpawnPoint.h"
#include "GameFramework/Actor.h"
#include "MonsterFactory.generated.h"

USTRUCT(BlueprintType)
struct FSpawnPoint
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform SpawnTransform;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMonsterSpawnPointType SpawnType;
	bool IsValid() const;
};

UCLASS()
class BLASTER_API AMonsterFactory : public AActor
{
	GENERATED_BODY()
	
public:	
	AMonsterFactory();
	
	ACharacter* SpawnMonster(EMonsterSpawnPointType SpawnPointType);
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monsters")
	TArray<TSubclassOf<ACharacter>> StreetMonsters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monsters")
	TArray<TSubclassOf<ACharacter>> HouseMonsters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monsters")
	TArray<TSubclassOf<ACharacter>> SewerMonsters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monsters")
	TArray<TSubclassOf<ACharacter>> ParkMonsters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monsters")
	TArray<TSubclassOf<ACharacter>> MiscMonsters;

	TArray<TSubclassOf<ACharacter>> AllMonsters;

private:
	UPROPERTY()
	TArray<FSpawnPoint> SpawnPoints;

	void CollectAllMonsterSpawns();
	void ShuffleSpawnPoints();
	FSpawnPoint FindUsableMonsterSpawn();
	TSubclassOf<ACharacter> ChooseRandomMonster(EMonsterSpawnPointType SpawnType = EMonsterSpawnPointType::ANY);
};
