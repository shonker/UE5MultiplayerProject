// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TubeSlide.generated.h"

UCLASS()
class BLASTER_API ATubeSlide : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATubeSlide();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadWrite, Replicated)
		bool bAttentive;
	UPROPERTY(BlueprintReadWrite, Replicated)
		bool bAttacking;
	
	UPROPERTY(BlueprintReadWrite, Replicated)
	FVector_NetQuantize AttackTarget;

	int32 CharactersInAttentionRadius = 0;
	int32 CharactersInAttackRadius = 0;


	class ABlasterCharacter* TargetCharacter;

	UPROPERTY(EditAnywhere, Category = "Worm Mesh")
		USkeletalMeshComponent* WormMesh;
	UPROPERTY(EditAnywhere, Category = "Tube Slide")
		UStaticMeshComponent* TubeSlide;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class USphereComponent* DamageSphere;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class USphereComponent* AttentionSphere;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class USphereComponent* AttackSphere;

	void AggroTimerFinished();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
		void OnDamageSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnAttentionSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnAttentionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
		void OnAttackSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	FTimerHandle AggroTimer;
};