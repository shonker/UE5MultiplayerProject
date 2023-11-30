// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Limb.generated.h"

UCLASS()
class BLASTER_API ALimb : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ALimb();
	// Called to bind functionality to input
	virtual void Tick(float DeltaTime) override;

	void OnBeginHitTimer();

	void ClampLinearVelocity();

	void PlayImpactSound();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	//virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	UFUNCTION(Server, Unreliable)
	void ServerMoveForward(float Value, FVector_NetQuantize10 AimDirection);
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastMoveForward(FVector_NetQuantize10 AimDirection);

    void MoveRight(float Value);
	UFUNCTION(Server, Unreliable)
	void ServerMoveRight(float Value);
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastMoveRight(float Value);


    void Turn(float Value);
    void LookUp(float Value);
	
	void Jump();
	UFUNCTION(Server, Reliable)
	void ServerJump();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastJump();
	
	void FireButtonPressed();
    void EquipButtonPressed();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector ForwardImpulse;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere, Category = Physics)
	float BaseLimbAccelerationForce = 170.f;

	float ActiveLimbAccelerationForce = BaseLimbAccelerationForce;

	bool Accelerating = false;

private:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* LimbMesh;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	FVector CurrentLinearVelocity;

	UPROPERTY(EditAnywhere, Category = Sound)
	class USoundCue* SplatSound;
	UPROPERTY(EditAnywhere, Category = Sound)
	class USoundCue* SurfaceImpactSound;
	
	UPROPERTY(EditAnywhere, Category = Sound)
	float	SplatNoiseAccelerationThreshold = 1000.f;

	UPROPERTY(EditAnywhere, Category = Physics)
	float MaxLinearVelocity = 1000.f;
	
	UPROPERTY(EditAnywhere, Category = Physics)
	float MaxImpulsableSpeed = 200.f;

	UPROPERTY(ReplicatedUsing = OnRep_bOnBeginHit)
	bool bOnBeginHit = false;

	int RestAfterHitDuration = 10;
	int RestAfterHitRemaining = 10;

	//rep vars can ONLY have input params of the var being repd
	//what gets passed in? the last var, BEFORE the update to the var :)
	UFUNCTION()
	void OnRep_bOnBeginHit(bool bNotOnBeginHit);
	
};
