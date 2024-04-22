#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterManager.generated.h"

/**
 * AMonsterManager handles the dynamic spawning and tracking of monsters in the game world based on the match progression.
 */
UCLASS()
class BLASTER_API AMonsterManager : public AActor
{
	GENERATED_BODY()

public:
	AMonsterManager();

	virtual void BeginPlay() override;
	void SpawnMonsterFactory();
	void ValidateExistingMonsters();

	void CheckAndSpawnMonsters();

	void SpawnMonsters(int Count);

	void UpdateMonsterCountTargets(float IntensityFactor);

private:
	FTimerHandle MonsterCheckTimer;

	UPROPERTY()
	TArray<ACharacter*> ActiveMonsters;

	UPROPERTY(EditAnywhere, Category = "Monster Management")
	TSubclassOf<AActor> MonsterFactoryBP;

	UPROPERTY()
	class AMonsterFactory* MonsterFactory;

	UPROPERTY(EditAnywhere, Category = "Monster Management")
	int32 MaxMonsterCount;

	UPROPERTY(EditAnywhere, Category = "Monster Management")
	int32 MaxSimultaneouslySpawnedCount;

	float CurrentTargetMonsterCount;

	float GameDuration;

	void AdjustMonsterTargets();
};
