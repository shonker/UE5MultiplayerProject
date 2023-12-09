// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
	
public:
	virtual void Fire(const FVector& HitTarget) override;

protected:
	UFUNCTION()
		FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);
		
	UPROPERTY(EditAnywhere)
		float Damage = 20.f;

	UPROPERTY(EditAnywhere)
		class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactEnvironmentParticles;

	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactCharacterParticles;

	UPROPERTY(EditAnywhere)
		class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
		class USoundCue* ImpactEnvironmentSound;

	UPROPERTY(EditAnywhere)
		class USoundCue* ImpactCharacterSound;

	UPROPERTY(EditAnywhere)
		UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
		UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
		USoundCue* FireSound;

private:

	/*
	Trace End W/ Scatter
	*/
	UPROPERTY(EditAnywhere, Category = "WeaponScatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "WeaponScatter")
	float SphereRadius = 75.f;

	UPROPERTY(EditAnywhere, Category = "WeaponScatter")
	bool bUseScatter = false;

	bool FleshImpact = false;
};
