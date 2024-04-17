// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcItemManager.h"
#include "Item.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "UObject/UObjectIterator.h"

void AProcItemManager::ProcGen() 
{
	Super::ProcGen();
	FindItemSpawns();
	AssignPredeterminedLocations();
	CalculateItemDistribution();
	SpawnItems();
}

void AProcItemManager::FindItemSpawns()
{
	for (TActorIterator<AItemSpawnPoint> It(GetWorld()); It; ++It)
	{
		AItemSpawnPoint* SpawnPoint = *It;
		if (SpawnPoint)
		{
			SpawnPoints.Add(SpawnPoint);
		}
	}
}
void AProcItemManager::AssignPredeterminedLocations()
{
    for (int32 i = SpawnPoints.Num() - 1; i >= 0; --i)
    {
        AItemSpawnPoint* SpawnPoint = SpawnPoints[i];
        if (!SpawnPoint) continue;

        TArray<FSpawnableItems>* RelevantItems = nullptr;
        TArray<int> CurseLevelProbabilities; // Stores cumulative probabilities for CurseLevels
        ECurseLevel SelectedCurseLevel = ECurseLevel::Curse0; // Default CurseLevel if none is applied
        

        switch (SpawnPoint->SpawnPointType)
        {
            case EItemType::CURSED:
                RelevantItems = &SpawnableAlwaysCursed;
                CurseLevelProbabilities = {0, 40, 70, 80, 100}; // Weights for Curse0, Curse1, Curse2, Curse3
                break;
            case EItemType::EXTRACURSED:
                RelevantItems = &SpawnableSometimesCursed;
                SelectedCurseLevel = ECurseLevel::Curse3; // Directly adds CurseLevel ECurseLevel::Curse3 with 100% certainty
                CurseLevelProbabilities = {0, 0, 0, 0, 100}; // Only Curse3
                break;
            case EItemType::MASK:
                RelevantItems = &SpawnableMasks;
                break;
            case EItemType::WEAPON:
                RelevantItems = &SpawnableWeapons;
                break;
            default:
                continue; // Skip if it doesn't match any known type
        }

        if (RelevantItems && RelevantItems->Num() > 0)
        {
            int PointsToAdd = 0;
            const int ItemIndex = RS.RandRange(0, RelevantItems->Num() - 1);
            
            // Only calculate CurseLevel if probabilities are defined
            if (CurseLevelProbabilities.Num() > 0)
            {
                const int CurseLevelRand = RS.RandRange(0, 99);
                if (CurseLevelRand < CurseLevelProbabilities[0])
                    SelectedCurseLevel = ECurseLevel::Curse0;
                else if (CurseLevelRand < CurseLevelProbabilities[1])
                    SelectedCurseLevel = ECurseLevel::Curse1;
                else if (CurseLevelRand < CurseLevelProbabilities[2])
                    SelectedCurseLevel = ECurseLevel::Curse2;
                else
                    SelectedCurseLevel = ECurseLevel::Curse3;
                
                PointsToAdd = GetPredeterminedPointsForCurseLevel(SelectedCurseLevel);
            }

            SelectedSpawns.Add(FItemsToSpawn{SpawnPoint, (*RelevantItems)[ItemIndex].ItemClass, SelectedCurseLevel});
            TotalSpawnedCurseTally += PointsToAdd;
        }

        SpawnPoints.RemoveAt(i); 
    }
}

int AProcItemManager::GetPredeterminedPointsForCurseLevel(const ECurseLevel& CurseLevel)
{
    if (CurseLevel == ECurseLevel::Curse1)
        return 6;
    else if (CurseLevel == ECurseLevel::Curse2)
        return 12;
    else if (CurseLevel == ECurseLevel::Curse3)
        return 25;
    return 0; // ECurseLevel::Curse0 or default case
}


void AProcItemManager::CalculateItemDistribution()
{
	 TArray<int> CurseLevelProbabilities; // Stores cumulative probabilities for CurseLevels
    while (TotalSpawnedCurseTally < CurseTallyNecessaryToWinLevel && SpawnPoints.Num() > 0)
    {
        const int Index = RS.RandRange(0, SpawnPoints.Num() - 1);
        AItemSpawnPoint* ChosenSpawnPoint = SpawnPoints[Index];
        SpawnPoints.RemoveAt(Index);

        const int RandPercentage = RS.RandRange(0, 99); 
        TArray<FSpawnableItems>* SelectedArray = nullptr;

        if (RandPercentage < 50) // 50% chance for an ANY item
        {
            SelectedArray = (RS.RandRange(0, 99) < 80) ? &SpawnableSometimesCursed : &SpawnableAlwaysCursed;
			if (SelectedArray == &SpawnableSometimesCursed)
			{
				CurseLevelProbabilities = {50, 80, 95, 100};  //Curse0, Curse1, Curse2, Curse3
			}
        	if (SelectedArray == &SpawnableAlwaysCursed)
        	{
        		CurseLevelProbabilities = {0, 50, 90, 100};  //Curse0, Curse1, Curse2, Curse3
        	}
        }
        else if (RandPercentage < 90) // 40% chance for always cursed
        {
            SelectedArray = &SpawnableAlwaysCursed;
            CurseLevelProbabilities = {0, 50, 90, 100}; //Curse0, Curse1, Curse2, Curse3
        }
        else // 10% chance, always cursed
        {
            SelectedArray = (RS.RandRange(0, 99) < 50) ? &SpawnableSometimesCursed : &SpawnableAlwaysCursed;
            CurseLevelProbabilities = {0, 0, 0, 100};  //Curse0, Curse1, Curse2, Curse3
        }

        if (SelectedArray && SelectedArray->Num() > 0)
        {
            const int ItemIndex = RS.RandRange(0, SelectedArray->Num() - 1);
            const int CurseLevelRand = RS.RandRange(0, 99);
            ECurseLevel SelectedCurseLevel;
            if (CurseLevelRand < CurseLevelProbabilities[0])
                SelectedCurseLevel = ECurseLevel::Curse0;
            else if (CurseLevelRand < CurseLevelProbabilities[1])
                SelectedCurseLevel = ECurseLevel::Curse1;
            else if (CurseLevelRand < CurseLevelProbabilities[2])
                SelectedCurseLevel = ECurseLevel::Curse2;
            else
                SelectedCurseLevel = ECurseLevel::Curse3;

            const int PointsToAdd = GetPointsForCurseLevel(SelectedCurseLevel);
            SelectedSpawns.Add(FItemsToSpawn{ChosenSpawnPoint, (*SelectedArray)[ItemIndex].ItemClass, SelectedCurseLevel});
            TotalSpawnedCurseTally += PointsToAdd;
        }
    }

    // Adjust the curse tally necessary to win if not enough points were accumulated
    if (TotalSpawnedCurseTally < CurseTallyNecessaryToWinLevel)
    {
        CurseTallyNecessaryToWinLevel = FMath::Max(30, TotalSpawnedCurseTally / 2);
    }
}

int AProcItemManager::GetPointsForCurseLevel(const ECurseLevel& CurseLevel)
{
	if (CurseLevel == ECurseLevel::Curse1)
		return 7;
	else if (CurseLevel == ECurseLevel::Curse2)
		return 17;
	else if (CurseLevel == ECurseLevel::Curse3)
		return 33;
	return 0; 
}

void AProcItemManager::SpawnItems()
{
	int32 Curse0Tally = 0;
	int32 Curse1Tally = 0;
	int32 Curse2Tally = 0;
	int32 Curse3Tally = 0;
	for (const FItemsToSpawn& Spawn : SelectedSpawns)
	{
	    FVector Location = Spawn.SpawnPoint->GetActorLocation();
		FRotator Rotation = Spawn.SpawnPoint->GetActorRotation();
	    AAProcActor* SpawnedProcActor = SpawnAt(Spawn.ItemToSpawn, Location , Rotation);
	    AItem* SpawnedItem = Cast<AItem>(SpawnedProcActor);
	    if (SpawnedItem)
	    {
		    SpawnedItem->CurseLevel = Spawn.CurseLevel;
			switch (Spawn.CurseLevel)
			{
			case ECurseLevel::Curse0:
				Curse0Tally++;
				break;
			case ECurseLevel::Curse1:
				Curse1Tally++;
				break;
			case ECurseLevel::Curse2:
				Curse2Tally++;
				break;
			case ECurseLevel::Curse3:
				Curse3Tally++;
				break;
		    default:
			    UE_LOG(LogTemp, Error, TEXT("Item with improper or no curse value"));
		    	break;
			}
	    }
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Curse0Tally: %i, Curse1Tally: %i, Curse2Tally: %i, Curse3Tally: %i"), Curse0Tally, Curse1Tally, Curse2Tally, Curse3Tally);
}
