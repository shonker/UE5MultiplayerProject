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
	
    

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
	void Jump();
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
