// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RagdollCube.generated.h"

UCLASS()
class BLASTER_API ARagdollCube : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARagdollCube();
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
		class UBoxComponent* PhysicsBox;
	UFUNCTION()
		void OnPhysicsBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnPhysicsBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	bool bFollowPlayer = false;
	class ABlasterCharacter* FollowChar;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	USkeletalMeshComponent* CharacterMesh;

public:
	void SetCharacterMesh(USkeletalMeshComponent* Mesh);

};
