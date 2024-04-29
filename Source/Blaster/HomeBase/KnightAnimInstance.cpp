// Fill out your copyright notice in the Description page of Project Settings.


#include "KnightAnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "GameFramework/GameMode.h"

void UKnightAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	APlayerController* LocalPC = UGameplayStatics::GetPlayerController(this, 0);
	BlasterPC = Cast<ABlasterPlayerController>(LocalPC);

	if (BlasterPC)
	{
		MatchState = BlasterPC->GetMatchState();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerControllerNotFound for Knight AnimInstance"));
	}
	
}

void UKnightAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!bIsGameInProgress)
	{
		if (MatchState == MatchState::InProgress)
		{
			bIsGameInProgress = true;
			CountdownTime = BlasterPC->GetServerTime();

			//todo: evenly distribute the knight animation states via the time returned above. Animation (stab2) finishes @ moment server time = 0
		}
	}
}
