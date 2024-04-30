// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "KnightAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UKnightAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	
	FName MatchState;

	void IncrementAnimation();
	FTimerHandle AnimationIncrementTimer;

public:
	UPROPERTY(BlueprintReadWrite)
	int32 AnimStage = 0;
	UPROPERTY(BlueprintReadWrite)
	bool bIsHeld = false;
	UPROPERTY(BlueprintReadWrite)
	bool bIsInserted = false;
	
	UPROPERTY(BlueprintReadWrite)
	bool bResetKnight = false;
	
	UFUNCTION()
	void ActivateKnight(float MatchTime);
	
	UFUNCTION(BlueprintCallable)
	void ResetKnight();

	UFUNCTION(BlueprintImplementableEvent)
	void StartShaking();
	UFUNCTION(BlueprintImplementableEvent)
	void StopShaking();
};
