// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SwordAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API USwordAnimInstance : public UAnimInstance
{
	GENERATED_BODY()


public:
	int32 AnimStage = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bIsInserted = true;
	
	UPROPERTY(BlueprintReadWrite)
	bool bIsHeld = false;
	
	FTimerHandle AnimationIncrementTimer;
	void IncrementAnimation();
	void ActivateSword(float MatchTime);
};
