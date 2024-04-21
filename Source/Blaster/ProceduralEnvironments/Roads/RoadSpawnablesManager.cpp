// Fill out your copyright notice in the Description page of Project Settings.


#include "RoadSpawnablesManager.h"

#include "EngineUtils.h"
#include "Engine/World.h"
#include "UObject/UObjectIterator.h"

void ARoadSpawnablesManager::ProcGen() 
{
	Super::ProcGen();
	FindRoadSpawns();
	PopulateObjectsToSpawnArray();
	SpawnRoadObjects();
}

void ARoadSpawnablesManager::FindRoadSpawns()
{
	for (TActorIterator<ARoadSpawnPoint> It(GetWorld()); It; ++It)
	{
		ARoadSpawnPoint* SpawnPoint = *It;
		if (SpawnPoint)
		{
			switch (SpawnPoint->SpawnPointType)
			{
			case ESpawnPointType::Sidewalk:
				SidewalkSpawnPoints.Add(SpawnPoint);
				break;
			case ESpawnPointType::MiddleOfRoad:
				MiddleOfRoadSpawnPoints.Add(SpawnPoint);
				break;
			default:

				break;
			}
		}
	}
}

void ARoadSpawnablesManager::PopulateObjectsToSpawnArray()
{
	DetermineRoadLightsAndSpawnPoints();
	DetermineRoadObjectsAndSpawnPoints();
}

void ARoadSpawnablesManager::DetermineRoadObjectsAndSpawnPoints()
{
	const int32 RoadObjectMaxCount = RS.RandRange(5, 15);
	int32 RoadObjectCount = 0;
	while (RoadObjectCount < RoadObjectMaxCount)
	{
		RoadObjectCount++;
		bool bMiddleOfRoadObjects = RS.RandRange(0,1) == 1;
		TArray<ARoadSpawnPoint*> SelectedSpawnPointArray =  bMiddleOfRoadObjects? MiddleOfRoadSpawnPoints: SidewalkSpawnPoints;
		if (SelectedSpawnPointArray.Num() <= 0)
		{
			continue;
		}
		
		TArray<TSubclassOf<AAProcActor>>* SelectedObjectArray = nullptr;
		SelectedObjectArray =  bMiddleOfRoadObjects? &MiddleOfTheRoadObjects : &SidewalkRoadObjects;
		if (SelectedObjectArray->Num() == 0)
		{
			continue;
		}
		const int SpawnIndex = RS.RandRange(0, SidewalkSpawnPoints.Num() - 1);
		ARoadSpawnPoint* ChosenSpawnPoint = SidewalkSpawnPoints[SpawnIndex];
		SidewalkSpawnPoints.RemoveAt(SpawnIndex);
    	
		const int ObjIndex = RS.RandRange(0, SelectedObjectArray->Num() - 1); // Correct index calculation

		SelectedSpawns.Add(FSelectedRoadObjects{ChosenSpawnPoint, (*SelectedObjectArray)[ObjIndex]});
		
	}
}

void ARoadSpawnablesManager::DetermineRoadLightsAndSpawnPoints()
{
	const int32 RoadLightsMaxCount = RS.RandRange(5,12);
	int32 RoadLightsCount = 0;
	
	while (RoadLightsCount < RoadLightsMaxCount && SidewalkSpawnPoints.Num() > 0)
	{
		RoadLightsCount++;
		TArray<TSubclassOf<AAProcActor>>* SelectedArray = nullptr;
		SelectedArray = &RoadLightSources;

		const int SpwnIndex = RS.RandRange(0, SidewalkSpawnPoints.Num() - 1);
		ARoadSpawnPoint* ChosenSpawnPoint = SidewalkSpawnPoints[SpwnIndex];
		SidewalkSpawnPoints.RemoveAt(SpwnIndex);
    	
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
			SpawnedProcActor->ProcGen();
		}
	} 
}
