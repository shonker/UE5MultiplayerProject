// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:	
	UCombatComponent();

	//this gives UBC full access to combat comp
	friend class ABlasterCharacter;

	//often times, fwd declarations are done above UCLASS, but i don't know enough about that to have an opinion
	void EquipWeapon(class AWeapon* WeaponToEquip);

protected:
	virtual void BeginPlay() override;

private:
	//create ABC char class so we don't have to constantly cast
	//has 2 b set as early as possible
	class ABlasterCharacter* Character;
	AWeapon* EquippedWeapon;

public:	

		
};
