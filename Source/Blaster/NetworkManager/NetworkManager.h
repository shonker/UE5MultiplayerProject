// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetworkManager.generated.h"

//
//USTRUCT()
//struct FMyTrackedItem : public FFastArraySerializerItem {
//	GENERATED_BODY()
//
//		// Your properties (e.g., a weak pointer to the tracked actor)
//};
//
//USTRUCT()
//struct FMyTrackedItems : public FFastArraySerializer {
//	GENERATED_BODY()
//
//		TArray<FMyTrackedItem> Items;
//
//	// Implement NetDeltaSerialize
//};

UCLASS()
class BLASTER_API ANetworkManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANetworkManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
