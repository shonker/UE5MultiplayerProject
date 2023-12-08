// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketMovementComponent.h"

//fully qualify the enum
URocketMovementComponent::EHandleBlockingHitResult 
URocketMovementComponent::HandleBlockingHit(
	const FHitResult& Hit,
	float TimeTick,
	const FVector& MoveDelta,
	float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(
		Hit,
		TimeTick,
		MoveDelta,
		SubTickTimeRemaining
	);
	return EHandleBlockingHitResult::AdvanceNextSubstep;
}
void URocketMovementComponent::HandleImpact(
	const FHitResult& Hit, 
	float TimeSlice, 
	const FVector& MoveDelta)
{
	//nothing beeyatch
	//rockets should not stop, only xplode when collision box detects hit
}
