// Fill out your copyright notice in the Description page of Project Settings.


#include "AProcActor.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"

// Sets default values
AAProcActor::AAProcActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

bool AAProcActor::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	if (const ABlasterPlayerController* PC = Cast<ABlasterPlayerController>(RealViewer))
	{
		return PC->bClientFinishedProceduralGeneration && Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
	}
	return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
}

// Called when the game starts or when spawned
void AAProcActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AAProcActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

