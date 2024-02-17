// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/ProceduralEnvironments/AProcActor.h"
#include "CurvedTower.h"
#include "CTowerPlatform.generated.h"

/**
 * 
 */

UCLASS()
class BLASTER_API ACTowerPlatform : public AAProcActor
{
	GENERATED_BODY()
	

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation")
		TArray<FTransform> OutputTransforms;

	virtual void ProcGen() override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Procedural Generation")
	TArray<FObjectTypeInfo> TowerTypes;

private:

};
