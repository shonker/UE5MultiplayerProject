// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
	

public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDebt(float Debt);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdownText(float CountdownTime);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
private:
	class ABlasterHUD* BlasterHUD;

	float MatchTime = 120.f;
};
