// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Weapon/Weapon.h"
#include "Item.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AItem : public AWeapon
{
	GENERATED_BODY()
	
public:


private:
	virtual void SpendRound() override;
	virtual void Fire(const FVector& HitTarget) override;

public:
	virtual void SetWeaponState(EWeaponState State) override;
};
