// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
//ignore the weapon wordage tech debt

void AItem::SpendRound()
{
	//do nothing, no ammo in this context
	//todo: some items may have "uses"(?)
}

void AItem::Fire(const FVector& HitTarget)
{
	//optionally set the weap related fire vars (projectile, animation) in editor
	//default animation proably "shake" (?)
	Super::Fire(HitTarget);

	//todo: add weird "shaken" or whatnot effects
}

void AItem::SetWeaponState(EWeaponState State)
{
	Super::SetWeaponState(State);

	//todo: add unique item effects from being picked up/ dropped

	switch (State)
	{
	case EWeaponState::EWS_Equipped:

		break;

	case EWeaponState::EWS_Dropped:

		break;

	case EWeaponState::EWS_Stored:

		break;
	}
}
