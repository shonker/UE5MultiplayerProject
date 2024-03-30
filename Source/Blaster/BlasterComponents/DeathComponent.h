// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DeathComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UDeathComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDeathComponent();

	//this gives ABC full access to death comp
	friend class ABlasterCharacter;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	class ARagdollCube* SpawnedPhysicsBox;
	UPROPERTY(EditAnywhere, Category = Elim)
	TSubclassOf<class ARagdollCube> PhysicsBoxBlueprint;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
		class ABlasterCharacter* Character;
	UPROPERTY()
		class ABlasterPlayerController* Controller;
	UPROPERTY()
		class UCombatComponent* Combat;
	UPROPERTY()
		class ABlasterHUD* HUD;
	
	void TransitionToSpectateCamera();
	float CameraTransitionDelay = 4.f;
	void TransitionToLimb();
};
