// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
//see under UCLASS where we also inherit the interface
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABlasterCharacter();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMontage();
	void PlayThrowMontage();

	virtual void OnRep_ReplicatedMovement() override;

	void Elim();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	//this is defined in the myblastercharacter event graph
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

protected:
	virtual void Destroyed() override;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//movement funcs
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void ReloadButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn();
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	void PlayHitReactMontage();
	void HandleDeathTransition();
	void ThrowButtonPressed();
	void ThrowButtonReleased();
	
	//ufunc necessary for callback frunctionos
	UFUNCTION()
	void ReceiveDamage(
		AActor* DamagedActor, 
		float Damage, 
		const UDamageType* DamageType,
		class AController* InstigatorController,
		AActor* DamageCauser
	);
	void UpdateHUDHealth();
	//Poll for any relevant classes and initialize HUD
	void PollInit();
	void RotateInPlace(float DeltaTime);

private:
	UPROPERTY(EditAnywhere, Category = Elim)
	TSubclassOf<class ALimb> LimbClass;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FirstPersonCamera;

	//meta specifier allows private variables to be blueprintreadonly
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	//makes it so only when this is updated on server, it is repped on clients
	//replication only works when the var is changed
	//also repusing = onrep_overWeap sets or_ol up to be called when the var changes
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	class USphereComponent* InteractSphere;

	//rep vars can ONLY have input params of the var being repd
	//what gets passed in? the last var, BEFORE the update to the var :)
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	//meta specifier allows private variables to be blueprintreadonly
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	UPROPERTY(EditAnywhere, Category = Combat)
	UParticleSystem* RedDotParticles;

	class UParticleSystemComponent* RedDotEmitter;

	FVector_NetQuantize InteractTargetLocation;

	UFUNCTION(Server, Reliable)
	void ServerSetInteractTarget(FVector_NetQuantize InteractTarget);

	UFUNCTION(Server, Unreliable)
	void ServerEquipButtonPressed();

	//protected aim offset var
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	UPROPERTY()
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);
	
	/*
	Animation Montages
	*/

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ReloadMontage;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowMontage;

	UPROPERTY(EditAnywhere, Category = Elim)
	UParticleSystem* DeathParticles;

	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 70.f;
	
	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	/*
	Player Health
	*/

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "PlayerStats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;

	bool bElimmed = false;
	FTimerHandle ElimTimer;
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 1.5f;
	void ElimTimerFinished();

	/*
	Dissolve effect
	*/
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();
	
	//Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	//material instance set on the blueprint, used w/ the dynamic material instance
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;	
	
	//material instance set on the blueprint, used w/ the dynamic material instance
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* InvisibleMaterialInstance;

	UPROPERTY(EditAnywhere, Category = Elim)
	class USoundCue* DeathSound;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;
public:	
	//here it is updated for all clients AND server (logic for that inside)
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAOYaw() const { return AO_Yaw; }
	FORCEINLINE float GetAOPitch() const { return AO_Pitch; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const {return TurningInPlace; }
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const {return FollowCamera; }
	//gun rotation for hands
	UPROPERTY(EditAnywhere, Category = "Weapon Rotation Correction")
	float RightHandRotationRoll;
	UPROPERTY(EditAnywhere, Category = "Weapon Rotation Correction")
	float RightHandRotationYaw;
	UPROPERTY(EditAnywhere, Category = "Weapon Rotation Correction")
	float RightHandRotationPitch;
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetHeath()  const { return Health; }
	FORCEINLINE float GetMaxHealth()  const { return MaxHealth; }
	FORCEINLINE ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
};
