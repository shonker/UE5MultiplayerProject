// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Buttons/MyButton.h"
#include "Components/ChildActorComponent.h"

#include "Door.generated.h"

UCLASS()
class BLASTER_API ADoor : public AActor
{
	GENERATED_BODY()
	
public:	

	ADoor();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(EditAnywhere, Category = "Door Mesh")
	USkeletalMeshComponent* DoorMesh;

	UFUNCTION()
	void KnobButtonPress();
	UFUNCTION()
	void KnobButtonRelease();
	UFUNCTION()
	void LockButtonPress();
	UFUNCTION()
	void KnobButtonDraggedOff();

	/*UPROPERTY(EditAnywhere, Category = "Buttons")
	AMyButton* DoorKnobButton;

	UPROPERTY(EditAnywhere,Category = "Buttons")
	AMyButton* LockButton;*/

	UPROPERTY(EditAnywhere, Category = "Components")
		UChildActorComponent* DoorKnobButtonComponent;

	UPROPERTY(EditAnywhere, Category = "Components")
		UChildActorComponent* LockButtonComponent;

protected:

	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;
	UPROPERTY(BlueprintReadOnly)
	bool bIsOpen = false;
	UPROPERTY(BlueprintReadOnly)
	bool bKnobTurning = false;
	UPROPERTY(BlueprintReadOnly)
	bool bIsLocked = false;
	UPROPERTY(BlueprintReadWrite)
	bool bAttemptOpen = false;
	
};
