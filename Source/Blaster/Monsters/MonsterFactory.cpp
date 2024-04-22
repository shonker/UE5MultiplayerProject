// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterFactory.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "UObject/UObjectIterator.h"

AMonsterFactory::AMonsterFactory()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMonsterFactory::BeginPlay()
{
	Super::BeginPlay();
	CollectAllMonsterSpawns();
}

void AMonsterFactory::CollectAllMonsterSpawns()
{
	for (TActorIterator<AMonsterSpawnPoint> It(GetWorld()); It; ++It)
	{
		AMonsterSpawnPoint* SpawnPoint = *It;
		if (SpawnPoint)
		{
			FSpawnPoint NewSpawnPoint;
            
			NewSpawnPoint.SpawnTransform = SpawnPoint->GetActorTransform();
			NewSpawnPoint.SpawnType = SpawnPoint->SpawnPointType;
            
			SpawnPoints.Add(NewSpawnPoint);

			SpawnPoint->Destroy();
		}
	}
}

void AMonsterFactory::ShuffleSpawnPoints()
{
	if (SpawnPoints.Num() > 1)
	{
		int32 LastIndex = SpawnPoints.Num() - 1;
		for (int32 i = 0; i <= LastIndex; ++i)
		{
			int32 Index = FMath::RandRange(i, LastIndex);
			if (i != Index)
			{
				SpawnPoints.Swap(i, Index);
			}
		}
	}
}

FSpawnPoint AMonsterFactory::FindUsableMonsterSpawn()
{
	 // Shuffle the SpawnPoints to randomize the selection process
    ShuffleSpawnPoints();

    // Attempt to find a usable spawn point from the randomized list
	for (const FSpawnPoint& SpawnPoint : SpawnPoints)
	{
		bool bIsVisibleToAnyPlayer = false;
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (PC && PC->GetPawn())
			{
				FHitResult Hit;
				FVector Start = PC->GetPawn()->GetActorLocation();
				FVector End = SpawnPoint.SpawnTransform.GetLocation();
				FCollisionQueryParams Params;
				Params.AddIgnoredActor(PC->GetPawn());

				// Perform a line trace from player to spawn point
				if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
				{
					if (Hit.bBlockingHit && Hit.GetActor() != PC->GetPawn())
					{
						bIsVisibleToAnyPlayer = true;
						break;
					}
				}
			}
		}

		if (!bIsVisibleToAnyPlayer)
		{
			return SpawnPoint;
		}
	}
	
	FSpawnPoint InvalidSpawn;
	InvalidSpawn.SpawnType = EMonsterSpawnPointType::INVALID;
	return InvalidSpawn;
}

TSubclassOf<ACharacter> AMonsterFactory::ChooseRandomMonster(EMonsterSpawnPointType SpawnType)
{
	// Example to choose a random monster based on spawn type
	TArray<TSubclassOf<ACharacter>>* MonsterList = nullptr;
	switch (SpawnType)
	{
	case EMonsterSpawnPointType::STREET:
		MonsterList = &StreetMonsters;
		break;
	case EMonsterSpawnPointType::HOUSE:
		MonsterList = &HouseMonsters;
		break;
	case EMonsterSpawnPointType::SEWER:
		MonsterList = &SewerMonsters;
		break;
	case EMonsterSpawnPointType::PARK:
		MonsterList = &ParkMonsters;
		break;
	default:
		return nullptr;
	}

	if (MonsterList && MonsterList->Num() > 0)
	{
		int32 Index = FMath::RandRange(0, MonsterList->Num() - 1);
		return (*MonsterList)[Index];
	}

	return nullptr;
}

ACharacter* AMonsterFactory::SpawnMonster(EMonsterSpawnPointType SpawnPointType)
{
	//todo: incorporate specifiable spawnmonstertype, currently the arg does nothing and one is randomly selected
	FSpawnPoint SpawnPoint = FindUsableMonsterSpawn();
	if (SpawnPoint.IsValid())
	{
		TSubclassOf<ACharacter> MonsterToSpawn = ChooseRandomMonster(SpawnPoint.SpawnType);
		if (!MonsterToSpawn)
		{
			UE_LOG(LogTemp, Error, TEXT("SpawnMonster: No monster class provided"));
			return nullptr;
		}

		ACharacter* SpawnedMonster = GetWorld()->SpawnActor<ACharacter>(MonsterToSpawn, SpawnPoint.SpawnTransform);
		return SpawnedMonster;
	}
	UE_LOG(LogTemp, Error, TEXT("SpawnMonster: Invalid spawn point"));
	return nullptr;
}


bool FSpawnPoint::IsValid() const
{
	// Assume an invalid spawn type indicates an invalid spawn point
	return SpawnType != EMonsterSpawnPointType::INVALID;
}