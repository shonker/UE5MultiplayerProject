// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProcHouse.generated.h"

UCLASS()
class BLASTER_API AProcHouse : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProcHouse();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, Category = "Modular Components")
	UStaticMeshComponent* HouseMesh;
	UPROPERTY(EditAnywhere, Category = "Modular Components")
	UStaticMeshComponent* PlotMesh;
};
