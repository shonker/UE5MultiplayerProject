// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/ProceduralEnvironments/AProcActor.h"
#include "ItemSpawnPoint.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EItemType : uint8
{
	ANY,
	CURSED,
	EXTRACURSED,
	MASK,
	WEAPON
};

UCLASS()
class BLASTER_API AItemSpawnPoint : public AAProcActor
{
	GENERATED_BODY()

	public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemType SpawnPointType = EItemType::ANY;
	
};
