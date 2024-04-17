// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Weapon/Weapon.h"
#include "Item.generated.h"

/**
 * 
 */


UENUM(BlueprintType)
enum class ECurseLevel : uint8
{
	Curse0,
	Curse1,
	Curse2,
	Curse3
};

UCLASS()
class BLASTER_API AItem : public AWeapon
{
	GENERATED_BODY()
	
public:
	ECurseLevel CurseLevel = ECurseLevel::Curse0;
	
protected:
	virtual void BeginPlay() override;
	void PlaySoundCueAtRandomLocation(USoundCue* SoundCue);

	/*
		CURSES
	*/
	//HARD Curse3
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
		
	//MEDIUM Curse2
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

	//EASY Curse1
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

	//ACTIVATION TRAITS/INDICATORS on some Curse 2 and Curse 3
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CurseActivations)
		bool bSingleScreenShake = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CurseActivations)
		bool bHeavyBreath = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CurseActivations)
		bool bOrbOfLight = false;
	//handled in BP
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = CurseActivations)
		bool bLineofSightSound = false;

	/*
		CURSE SFX
	*/
	UPROPERTY(EditAnywhere, Category = CurseSFX)
		class USoundCue* MurmurCue;
	UPROPERTY(EditAnywhere, Category = CurseSFX)
		class USoundCue* KnockingCue;
	UPROPERTY(EditAnywhere, Category = CurseSFX)
		class USoundCue* ManLaughingCue;
	UPROPERTY(EditAnywhere, Category = CurseSFX)
		class USoundCue* WomanLaughingCue;
	UPROPERTY(EditAnywhere, Category = CurseSFX)
		class USoundCue* ChildLaughingCue;
	UPROPERTY(EditAnywhere, Category = CurseSFX)
		class USoundCue* WallBangCue;

	//line of sight:
	UPROPERTY(EditAnywhere, Category = CurseSFX)
		class USoundCue* HummingCue;
	UPROPERTY(EditAnywhere, Category = CurseSFX)
		class USoundCue* DroneCue;
	UPROPERTY(EditAnywhere, Category = CurseSFX)
		class USoundCue* BuzzingCue;

	//pain????
	UPROPERTY(EditAnywhere, Category = CurseSFX)
		class USoundCue* HealthSapCue;
	UPROPERTY(EditAnywhere, Category = CurseSFX)
		class USoundCue* LongBreathCue;

	class ABlasterCharacter* BlasterOwnerCharacter;
	class ABlasterPlayerController* BlasterOwnerController;
private:
	virtual void SpendRound() override;
	virtual void Fire(const FVector& HitTarget) override;
	
	/*
		CURSES
	*/
	//hard
	void HeavyJumping(EWeaponState State);
	void HealthSap(EWeaponState State);

	FTimerHandle HealthSapTimerHandle;
	void DecreaseHealth();

	void ReversedMovementControls(EWeaponState State);
	void ReversedLookControls(EWeaponState State);
	void ActivateKisser(EWeaponState State);

	//medium
	void Fetus(EWeaponState State);
	void Blindness(EWeaponState State);

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
	/**
	 * Plays a laughing sound at a random location near the BlasterOwnerCharacter.
	 *
	 * @param OptionalLaughType Specifies the type of laughing sound to play.
	 *                          -1: Randomly selects a laughing sound (default).
	 *                           0: Man laughing sound.
	 *                           1: Woman laughing sound.
	 *                           2: Child laughing sound.
	 */
	void PlayLaughingSound(int32 OptionalLaughType = 31);

	void ChangingFOV(EWeaponState State);

	FTimerHandle FOVTimerHandle;
	FTimerHandle FOVInterpHandle;
	void RandomizeFOV();
	
	void ScreenShake(EWeaponState State);
	FTimerHandle ScreenShakeTimerHandle;
	void TriggerScreenShake();

	/*
	 * vibe related effects
	 */
	UPROPERTY(EditAnywhere, Category = "Curse Blueprints")
	TSubclassOf<AActor> OrbOfLightBlueprint;
	UFUNCTION()
	void OrbOfLight();
	//nothing here yet

public:
	virtual void OnSetWeaponState(EWeaponState State) override;
	void UpdateCurse(EWeaponState State);

};
