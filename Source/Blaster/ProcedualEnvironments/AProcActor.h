// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AProcActor.generated.h"

UCLASS()
class BLASTER_API AAProcActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAProcActor();
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	uint32 PGI = 0;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FRandomStream RS;
};
