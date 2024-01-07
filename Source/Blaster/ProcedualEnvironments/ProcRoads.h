// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProcRoads.generated.h"

UCLASS()
class BLASTER_API AProcRoads : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* RoadMesh;

public:
	AProcRoads();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
		TArray<TSubclassOf<AActor>> SpawnableObjects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
		TArray<USceneComponent*> SpawnPoints;

protected:
	virtual void BeginPlay() override;

private:
	int32 ChooseSpawnCount(const TArray<int32>& Distribution);
	void SpawnObjectAtPoint(USceneComponent* SpawnPoint);

};
