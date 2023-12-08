// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnHit(bool bCharacterHit);

	UPROPERTY(Replicated)
	bool bCharacterWasHit;

protected:
	virtual void BeginPlay() override;

	void StartDestroyTimer();
	void DestroyTimerFinished();

	//component doing the hitting (col box), actor being hit, primitive (other component being hit), the normal of the surface being hit, const ref to something?
	//callbacks bound to hit events (oncomponenthit) have to be ufunctions to work
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	float Damage = 10.f;

	UPROPERTY()
		class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactEnvironmentParticles;

	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactCharacterParticles;

	UPROPERTY(EditAnywhere)
		class USoundCue* FireSound;

	UPROPERTY()
		class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
		class USoundCue* ImpactEnvironmentSound;

	UPROPERTY(EditAnywhere)
		class USoundCue* ImpactCharacterSound;
	UPROPERTY(EditAnywhere)
		class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* TrailSystem;

	UPROPERTY()
		class UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(VisibleAnywhere)
		class UProjectileMovementComponent* ProjectileMovementComponent;

	UFUNCTION()
		void SpawnTrailSystem();

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* ProjectileMesh;

private:

	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;
};
