// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "CombatComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	
	//this gives ABC full access to combat comp
	friend class ABlasterCharacter;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//often times, fwd declarations are done above UCLASS, but i don't know enough about that to have an opinion
	void EquipWeapon(class AWeapon* WeaponToEquip);
	void Reload();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowFinished();

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);
	
	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void Server_Reload();

	UFUNCTION()
	void HandleReload();
	int32 AmountToReload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	void Throw();
	void StartThrowCharging();

	UFUNCTION(Server, Reliable)
	void ServerThrow(FVector_NetQuantize10 ProvidedThrowVector);

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticastThrow(FVector_NetQuantize10 ProvidedThrowVector);

	void DropEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void PlayEquipWeaponSound();
	void ReloadEmptyWeapon();
private:
	//create ABC char class so we don't have to constantly cast
	//has 2 b set as early as possible
	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* Controller;
	UPROPERTY()
	class ABlasterHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	//all you would need for single player functionality
	//bool bAiming;

	//multiplayer functional v.v
	//note that it is "registered" in the DOREPLIFETIME in CC.cpp
	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	/*
		HUD and Crosshairs
	*/
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitTarget;

	FHUDPackage HUDPackage;

	/*
	Aiming and FOV
	*/
	//set to camera's base FOV in BeginPlay
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoominterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/*
	`	AutomaticFire
	*/
	FTimerHandle FireTimer;
	
	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	//carried ammo for currently equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;
	
	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 2;

	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 20;

	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 40;	
	
	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 4;
	
	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 4;

	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 4;

	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();

	void ThrowCharging();
	float ThrowForce = 0.f;
	float ThrowForceIncrement = 100.f;
	float ThrowForceMax = 1000.f;
	UPROPERTY()
	AWeapon* WeaponToThrow;

public:	
		
};
