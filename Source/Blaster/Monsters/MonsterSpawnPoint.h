// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/ProceduralEnvironments/AProcActor.h"
#include "MonsterSpawnPoint.generated.h"
/**
 * 
 */


UENUM(BlueprintType)
enum class EMonsterSpawnPointType : uint8
{
	ANY,
	INVALID,
	HOUSE,
	STREET,
	CURVED,
	SEWER,
	PARK
};

UCLASS()
class BLASTER_API AMonsterSpawnPoint : public AAProcActor
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMonsterSpawnPointType SpawnPointType = EMonsterSpawnPointType::STREET;
};
