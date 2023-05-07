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
	void MulticastMoveForward(float Value, FVector_NetQuantize10 AimDirection);

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

	

private:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* LimbMesh;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	
};
