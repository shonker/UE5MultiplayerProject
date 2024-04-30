// Fill out your copyright notice in the Description page of Project Settings.


#include "SwordAnimInstance.h"


void USwordAnimInstance::IncrementAnimation()
{
	AnimStage += 1;

	//if (AnimStage == 7) bIsHeld = true;
	if (AnimStage == 8) bIsInserted = true;

	if (AnimStage >= 9)
	{
		GetWorld()->GetTimerManager().ClearTimer(AnimationIncrementTimer);
	}
}

void USwordAnimInstance::ActivateSword(float MatchTime)
{
	float AnimIncrementRate = MatchTime / 7.f;
	GetWorld()->GetTimerManager().SetTimer(AnimationIncrementTimer, this, &USwordAnimInstance::IncrementAnimation, AnimIncrementRate, true, 1);
}
