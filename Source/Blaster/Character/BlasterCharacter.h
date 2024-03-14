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
	friend class UDeathComponent;
	// Sets default values for this character's properties
	ABlasterCharacter();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void ManageVisualInteractionTargetLocations();
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMontage();
	void PlayThrowMontage();
	void PlayKissMontage();

	virtual void OnRep_ReplicatedMovement() override;

	bool bElimmed = false;
	void Elim();

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	//this is defined in the myblastercharacter event graph
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	UFUNCTION(BlueprintImplementableEvent)
	void ChangeInteractionDotColor(bool bInteractionDetected);

	/*
		ITEMS
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UInventoryComponent* InventoryComponent;

	void ItemShuffleLeft();
	void ItemShuffleRight();
	UFUNCTION(Server, Reliable)
	void ServerShuffleItem(bool IsLeft);

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
	void HandleEquipButtonPressed();
	void EquipButtonReleased();
	void HandleEquipButtonReleased();
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
	void ClearHUDInteractText();
	//Poll for any relevant classes and initialize HUD
	void PollInit();
	void RotateInPlace(float DeltaTime);

private:
	UPROPERTY(EditAnywhere, Category = Elim)
	TSubclassOf<class ALimb> LimbClass;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Camera)
	class USpringArmComponent* FPSBoom;

	//meta specifier allows private variables to be blueprintreadonly
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingButton)
	class AMyButton* OverlappingButton;

	//for kissing
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingFriend)
	class ABlasterCharacter* OverlappingFriend;

	//for ragdoll dragging
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingBody)
	class ARagdollCube* OverlappingBody;

	
	UFUNCTION()
	void OnInteractSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnInteractSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION()
	void OnRep_OverlappingButton(AMyButton* LastButton);
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
	UFUNCTION()
	void OnRep_OverlappingBody(ARagdollCube* LastCube);
	UFUNCTION()
	void OnRep_OverlappingFriend();

	void Kiss(bool StartOrEnd);

	//meta specifier allows private variables to be blueprintreadonly
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UDeathComponent* Death;

	FVector_NetQuantize LastHitTarget = FVector();

	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	class USphereComponent* InteractSphere;
	
	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	class USphereComponent* VisualTargetSphere;

	void SetInteractAndVisualTargetSphereLocation(FVector_NetQuantize Target);

	UFUNCTION(Server, Unreliable)
	void ServerSetInteractTarget(FVector_NetQuantize InteractTarget);

	UPROPERTY(ReplicatedUsing = OnRep_InteractTargetLocation)
	FVector_NetQuantize InteractTargetLocation;

	UFUNCTION()
	void OnRep_InteractTargetLocation();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bInteractionPossible = false;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonReleased();

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

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* KissMontage;

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

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;

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

	/*
		CURSES: make sure to put modifiers in the "remove all curses" function;
	*/
	bool bTakesFallDamage = false;
	bool bReversedMovementControls = false;
	bool bReversedLookControls = false;
	bool bShiftMovementControls = false;
public:	
	void RemoveAllCurses();
	void SetShiftMovementControls(bool Shift);
	void SetReversedMovementControls(bool Reversed);
	void SetReversedLookControls(bool Reversed);
	void SetTakesFallDamage(bool TakesFallDamage);
	//here it is updated for all clients AND server (logic for that inside)
	void SetInteractionPossible(bool Possible);
	void SetOverlappingWeapon(AWeapon* Weapon);
	void SetOverlappingButton(AMyButton* Button);
	void SetOverlappingBody(ARagdollCube* Cube);
	void SetOverlappingFriend(ABlasterCharacter* Friend);
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
	ECombatState GetCombatState();
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
};
