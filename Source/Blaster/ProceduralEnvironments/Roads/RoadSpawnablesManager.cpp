// Fill out your copyright notice in the Description page of Project Settings.


#include "RoadSpawnablesManager.h"

#include "EngineUtils.h"
#include "Engine/World.h"
#include "UObject/UObjectIterator.h"

void ARoadSpawnablesManager::ProcGen() 
{
	Super::ProcGen();
	FindRoadSpawns();
	CalculateItemDistribution();
	SpawnRoadObjects();
}

void ARoadSpawnablesManager::FindRoadSpawns()
{
	for (TActorIterator<ARoadSpawnPoint> It(GetWorld()); It; ++It)
	{
		ARoadSpawnPoint* SpawnPoint = *It;
		if (SpawnPoint)
		{
			SpawnPoints.Add(SpawnPoint);
		}
	}
}

void ARoadSpawnablesManager::CalculateItemDistribution()
{
	TArray<int> CurseLevelProbabilities; // Stores cumulative probabilities for CurseLevels
	const int32 RoadObjectMaxCount = RS.RandRange(5,12);
	int32 RoadObjectCount = 0;
	while (RoadObjectCount < RoadObjectMaxCount && SpawnPoints.Num() > 0)
	{
		RoadObjectCount++;
		TArray<TSubclassOf<AAProcActor>>* SelectedArray = nullptr;
		SelectedArray = &CommonRoadObjects;

		const int SpwnIndex = RS.RandRange(0, SpawnPoints.Num() - 1);
		ARoadSpawnPoint* ChosenSpawnPoint = SpawnPoints[SpwnIndex];
		SpawnPoints.RemoveAt(SpwnIndex);
    	
		const int ObjIndex = RS.RandRange(0, SelectedArray->Num() - 1); // Correct index calculation

		SelectedSpawns.Add(FSelectedRoadObjects{ChosenSpawnPoint, (*SelectedArray)[ObjIndex]});
	}
}

void ARoadSpawnablesManager::SpawnRoadObjects()
{
	
	for (const FSelectedRoadObjects& Spawn : SelectedSpawns)
	{
		FVector Location = Spawn.SpawnPoint->GetActorLocation();
		FRotator Rotation = Spawn.SpawnPoint->GetActorRotation();
		AAProcActor* SpawnedProcActor = SpawnAt(Spawn.SelectedObject, Location , Rotation);
		if (SpawnedProcActor)
		{
			SpawnedProcActor->ProcGen();//as of now this wont do anything
		}
	} 
}
