// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/ProceduralEnvironments/AProcActor.h"
#include "RoadSpawnPoint.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class ESpawnPointType : uint8
{
	Sidewalk,
	MiddleOfRoad
};


UCLASS()
class BLASTER_API ARoadSpawnPoint : public AAProcActor
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	ESpawnPointType SpawnPointType = ESpawnPointType::Sidewalk;
};
