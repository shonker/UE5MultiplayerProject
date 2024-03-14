// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Weapon/Weapon.h"
#include "Item.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AItem : public AWeapon
{
	GENERATED_BODY()
	
public:

protected:
	virtual void BeginPlay() override;
	/*
		CURSES
	*/
	//HARD
	UPROPERTY(EditAnywhere, Category = Curses)
		bool bBlindness = false;
	UPROPERTY(EditAnywhere, Category = Curses)
		bool bHealthSap = false;
	UPROPERTY(EditAnywhere, Category = Curses)
		bool bReversedMovementControls = false;
	UPROPERTY(EditAnywhere, Category = Curses)
		bool bReversedLookControls = false;
	UPROPERTY(EditAnywhere, Category = Curses)
		bool bActivateKisser = false;
		
	//MEDIUM
	UPROPERTY(EditAnywhere, Category = Curses)
		bool bFetus = false;
	UPROPERTY(EditAnywhere, Category = Curses)
		bool bHeavyJumping = false;
	UPROPERTY(EditAnywhere, Category = Curses)
		bool bCountdownPain = false;
	UPROPERTY(EditAnywhere, Category = Curses)
		bool bFallDamage = false;
	UPROPERTY(EditAnywhere, Category = Curses)
		bool bShiftMovementControls = false;

	//EASY
	UPROPERTY(EditAnywhere, Category = Curses)
		bool bKnocking = false;
	UPROPERTY(EditAnywhere, Category = Curses)
		bool bMurmuring = false;
	UPROPERTY(EditAnywhere, Category = Curses)
		bool bLaughing = false;
	UPROPERTY(EditAnywhere, Category = Curses)
		bool bChangingFOV = false;
	UPROPERTY(EditAnywhere, Category = Curses)
		bool bScreenShake = false;
	UPROPERTY(EditAnywhere, Category = Curses)
		bool bCastsRedLight = false;
	UPROPERTY(EditAnywhere, Category = Curses)
		bool bAutomaticDecapitation = false;

	//ACTIVATION TRAITS/INDICATORS
	UPROPERTY(EditAnywhere, Category = CurseActivations)
		bool bSingleScreenShake = false;
	UPROPERTY(EditAnywhere, Category = CurseActivations)
		bool bHeavyBreath = false;
	UPROPERTY(EditAnywhere, Category = CurseActivations)
		bool bOrbOfLight = false;
	UPROPERTY(EditAnywhere, Category = CurseActivations)
		bool bLineofSightSFX = false;

	/*
		CURSE SFX
	*/
	UPROPERTY(EditAnywhere, Category = CurseSFX)
		class USoundCue* MurmurCue;
	UPROPERTY(EditAnywhere, Category = CurseSFX)
		class USoundCue* KnockingCue;
	UPROPERTY(EditAnywhere, Category = CurseSFX)
		class USoundCue* LaughingCue;

	//line of sight:
	UPROPERTY(EditAnywhere, Category = CurseSFX)
		class USoundCue* HummingCue;
	UPROPERTY(EditAnywhere, Category = CurseSFX)
		class USoundCue* DroneCue;
	UPROPERTY(EditAnywhere, Category = CurseSFX)
		class USoundCue* BuzzingCue;

private:
	virtual void SpendRound() override;
	virtual void Fire(const FVector& HitTarget) override;
	
	/*
		CURSES
	*/
	//hard
	void HeavyJumping(EWeaponState State);
	void Blindness(EWeaponState State);
	void HealthSap(EWeaponState State);

	FTimerHandle HealthSapTimerHandle;
	void DecreaseHealth();

	void ReversedMovementControls(EWeaponState State);
	void ReversedLookControls(EWeaponState State);
	void ActivateKisser(EWeaponState State);

	//medium
	void Fetus(EWeaponState State);

	void CountdownPain(EWeaponState State);
	FTimerHandle CountdownPainTimerHandle;
	void TriggerCountdownPain();

	void FallDamage(EWeaponState State);
	void ShiftMovementControls(EWeaponState State);

	//easy
	FTimerHandle KnockingSoundTimerHandle;
	void Knocking(EWeaponState State);
	void PlayKnockingSound();

	void Murmuring(EWeaponState State);
	FTimerHandle MurmuringSoundTimerHandle;
	void PlayMurmuringSound();

	void Laughing(EWeaponState State);
	FTimerHandle LaughingSoundTimerHandle;
	void PlayLaughingSound();

	void ChangingFOV(EWeaponState State);

	void ScreenShake(EWeaponState State);
	FTimerHandle ScreenShakeTimerHandle;
	void TriggerScreenShake();


public:
	virtual void OnSetWeaponState(EWeaponState State) override;
	void UpdateCurse(EWeaponState State);

};
