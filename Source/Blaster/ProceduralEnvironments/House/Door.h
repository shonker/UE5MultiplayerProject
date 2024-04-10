// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Buttons/MyButton.h"
#include "Blaster/ProceduralEnvironments/AProcActor.h"
#include "Components/ChildActorComponent.h"
#include "Door.generated.h"

UCLASS()
class BLASTER_API ADoor : public AAProcActor
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

	UPROPERTY(EditAnywhere, Category = "Components")
		UChildActorComponent* DoorKnobButtonComponent;

	UPROPERTY(EditAnywhere, Category = "Components")
		UChildActorComponent* LockButton;

	UFUNCTION()
	void OnRep_bIsOpen();
	
protected:

	virtual void BeginPlay() override;
	
public:	

	virtual void Tick(float DeltaTime) override;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_bIsOpen)
	bool bIsOpen = false;
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bKnobTurning = false;
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bIsLocked = false;
	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bAttemptOpen = false;

private:
	FTimerHandle TimerHandle_AttemptCast;
};
