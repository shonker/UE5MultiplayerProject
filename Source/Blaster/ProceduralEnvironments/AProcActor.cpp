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

void AAProcActor::ProcGen()
{
    //mostly for overriding to allow access to subclass functions 
    //without needing their includes
}

void AAProcActor::InitializePGI(int32* InPGI)
{
    PGI = InPGI;
}

AAProcActor* AAProcActor::SpawnAt(TSubclassOf<AActor> Actor, FVector& Location, FRotator& Rotation)
{
	if (!PGI) return nullptr;
	if (Actor == nullptr) return nullptr;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(*FString(Actor->GetName() + "_" + FString::FromInt(*PGI)));
	SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Required_ErrorAndReturnNull;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.bDeferConstruction = true;


 AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(Actor, Location, Rotation, SpawnParams);

    if (SpawnedActor)
    {
        if (SpawnedActor->IsA(AAProcActor::StaticClass()))
        {
            AAProcActor* SpawnedProcActor = Cast<AAProcActor>(SpawnedActor);
            SpawnedProcActor->InitializePGI(PGI);
            SpawnedProcActor->bNetStartup = true;
            SpawnedProcActor->Tags.Add(TEXT("ProcGen"));
            SpawnedProcActor->FinishSpawning(FTransform(Rotation, Location, FVector::OneVector));

            (*PGI)++;
            return SpawnedProcActor;
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Spawned actor is not of type AAProcActor. Actor class: %s"), *Actor->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Actor unable to spawn: %s"), *SpawnParams.Name.ToString());
    }
    return nullptr;
}
