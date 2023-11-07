// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Fountain.generated.h"

UCLASS()
class BLASTER_API AFountain : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFountain();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnFountainEntry();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnFountainEntry(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere)
	class UBoxComponent* FountainTrigger;

	void SprayBlood();

	UPROPERTY(EditAnywhere)
	UParticleSystem* BloodSprayParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* BloodSpraySound;

	bool bFountainEntered = false;

};
