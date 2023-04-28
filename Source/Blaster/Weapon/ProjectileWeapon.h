// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()
	
public: 

	virtual void Fire(const FVector& HitTarget) override;

private:
	//tsubclass allows us to populate this variable with aproj or any class derived from aproj
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;

};
