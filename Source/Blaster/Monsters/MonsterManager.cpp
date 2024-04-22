#include "MonsterManager.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "MonsterFactory.h"
#include "GameFramework/Character.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "GameFramework/GameStateBase.h"

AMonsterManager::AMonsterManager()
{
	GameDuration = 900.0f; // Default duration of 15 minutes, adjust as necessary.
	MaxMonsterCount = 50; // Set according to game design requirements.
	MaxSimultaneouslySpawnedCount = 5; // Example value, adjust as needed.
}

void AMonsterManager::BeginPlay()
{
	Super::BeginPlay();
	SpawnMonsterFactory();
	GetWorld()->GetTimerManager().SetTimer(MonsterCheckTimer, this, &AMonsterManager::CheckAndSpawnMonsters, 30.0f, true);
}

void AMonsterManager::SpawnMonsterFactory()
{
	if (!HasAuthority()) return;
	if (!MonsterFactoryBP) return;
	const FVector Location(0,0,0);
	const FRotator Rotation(0,0,0);
	MonsterFactory = Cast<AMonsterFactory>(GetWorld()->SpawnActor<AActor>(MonsterFactoryBP, Location, Rotation));
}

void AMonsterManager::ValidateExistingMonsters()
{
	for (int32 i = ActiveMonsters.Num() - 1; i >= 0; --i)
	{
		if (!ActiveMonsters[i] || !IsValid(ActiveMonsters[i]))
		{
			ActiveMonsters.RemoveAt(i);
		}
	}
}

void AMonsterManager::CheckAndSpawnMonsters()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode || GameMode->GetMatchState() != MatchState::InProgress)
	{
		return;
	}

	float RemainingTime = GameMode->GetCountdownTime();
	float TimeProgress = 1.0f - (RemainingTime / GameMode->MatchTime); 
	float IntensityFactor = FMath::Clamp(TimeProgress, 0.0f, 1.0f); 

	ValidateExistingMonsters();

	UpdateMonsterCountTargets(IntensityFactor);

	const int CurrentCount = ActiveMonsters.Num();
	const int MonstersToSpawn = FMath::Min(MaxSimultaneouslySpawnedCount, static_cast<int32>(CurrentTargetMonsterCount) - CurrentCount);

	if (MonstersToSpawn > 0)
	{
		SpawnMonsters(MonstersToSpawn);
	}
}

void AMonsterManager::UpdateMonsterCountTargets(float IntensityFactor)
{
	// Use the intensity factor to scale the target count
	CurrentTargetMonsterCount = FMath::Lerp(0.0f, static_cast<float>(MaxMonsterCount), IntensityFactor);
}

void AMonsterManager::SpawnMonsters(int Count)
{
	for (int i = 0; i < Count; ++i)
	{
		ACharacter* NewMonster = MonsterFactory->SpawnMonster(EMonsterSpawnPointType::ANY);
		if (NewMonster)
		{
		    ActiveMonsters.Add(NewMonster);
		}
	}
}


