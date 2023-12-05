// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();


protected:

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	void ActivateImpactSoundAndParticles(AActor* OtherActor);
	virtual void BeginPlay() override;
	void DestroyTimerFinished();

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	virtual void Destroyed() override;

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(EditAnywhere)
	USoundCue* ProjectileLoop;

	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;
private:
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* RocketMesh;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;

};
