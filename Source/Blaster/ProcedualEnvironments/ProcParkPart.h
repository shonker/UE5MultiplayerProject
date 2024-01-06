// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProcParkPart.generated.h"

UCLASS()
class BLASTER_API AProcParkPart : public AActor
{
	GENERATED_BODY()

public:
	AProcParkPart();

	// Output locations as editable vectors in the editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output Locations")
		TArray<FTransform> OutputTransforms;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
