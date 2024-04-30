// Fill out your copyright notice in the Description page of Project Settings.


#include "KnightAnimInstance.h"
#include "TimerManager.h"

void UKnightAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UKnightAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UKnightAnimInstance::IncrementAnimation()
{
	AnimStage += 1;
	if (AnimStage == 5) StartShaking();
	if (AnimStage == 7) bIsHeld = true;
	if (AnimStage == 8) {
		bIsInserted = true;
		StopShaking();
		}
	UE_LOG(LogTemp, Warning, TEXT("knight animation stage: %i"),AnimStage);
}

void UKnightAnimInstance::ActivateKnight(float MatchTime)
{
	float AnimIncrementRate = MatchTime / 7.f;
	GetWorld()->GetTimerManager().SetTimer(AnimationIncrementTimer, this, &UKnightAnimInstance::IncrementAnimation, AnimIncrementRate, true, 1);
	
	//set a timer here that increments int32 AnimStage by 1 for every CountdownTime/7, looping.
	//7 == number of animation points
			
	/*
	 *each new line is a pause between connected anims.
	 *game starts-> enter 0
	 *1111111 = 7 anim barriers;
	 *0: shake and turn head, no loop
	 *1 shake and fall down, no loop
	 *2 twist arm and continue into 3(loop 3, swinging back and forth)
	 *4 fall to table, cont into 5 (loop 5) 
	 *6 is looping, shaking while holding her
	 *7 gets up, palms her face to move to sword, cont into STAB1, cont into stab2idle (looping)
	 * stab2, end at end of anim & notities. a few seconds later perform "reset animation"
	 */
}

void UKnightAnimInstance::ResetKnight()
{
	GetWorld()->GetTimerManager().ClearTimer(AnimationIncrementTimer);
	AnimStage = 0;
	bResetKnight = true;
}


