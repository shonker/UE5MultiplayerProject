// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcFloor.h"
#include "GameFramework/Character.h"

// Sets default values
AProcFloor::AProcFloor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void AProcFloor::ProcGen()
{
    Super::ProcGen();
    if (bIsCeiling)
    {
        GenerateCeilingObjects();
    }
    else
    {
        GenerateFloorObjects();
    }
}

// Called when the game starts or when spawned
void AProcFloor::BeginPlay()
{
	Super::BeginPlay();
   // if (FMath::RandRange(0,10) == 1) SpawnNPCs();
}

void AProcFloor::GenerateCeilingObjects()
{
   const float LikelyHood = 0.07f;
    if (RS.FRand() <= LikelyHood)
    {
        if (SpawnableCeilingObjects.Num() > 0 && CeilingObjectTransforms.Num() > 0)
        {
            int32 ObjectIndex = RS.RandRange(0, SpawnableCeilingObjects.Num() - 1);
            int32 TransformIndex = RS.RandRange(0, CeilingObjectTransforms.Num() - 1);

            FTransform SpawnTransform = CeilingObjectTransforms[TransformIndex];
            FVector Location = SpawnTransform.GetLocation() + GetActorLocation(); 
            FRotator Rotation = SpawnTransform.GetRotation().Rotator(); 
            Rotation.Yaw -= 90.0f; 
            
            AAProcActor* ProcActor = SpawnAt(SpawnableCeilingObjects[ObjectIndex].ObjectClass, Location, Rotation);
            ProcActor->ProcGen();
            CeilingObjectTransforms.RemoveAt(TransformIndex);
        }
    }
}

void AProcFloor::GenerateFloorObjects()
{
    const float LikelyHood = 0.1f;
    if (RS.FRand() <= LikelyHood)
    {
        if (SpawnableFloorObjects.Num() > 0 && FloorObjectTransforms.Num() > 0)
        {
            int32 ObjectIndex = RS.RandRange(0, SpawnableFloorObjects.Num() - 1);
            int32 TransformIndex = RS.RandRange(0, FloorObjectTransforms.Num() - 1);

            FTransform SpawnTransform = FloorObjectTransforms[TransformIndex];
            FVector Location = SpawnTransform.GetLocation() + GetActorLocation(); 
            FRotator Rotation = SpawnTransform.GetRotation().Rotator(); 
            Rotation.Yaw -= 90.0f; 
            AAProcActor* ProcActor = SpawnAt(SpawnableFloorObjects[ObjectIndex].ObjectClass, Location, Rotation);
            ProcActor->ProcGen();
            
            CeilingObjectTransforms.RemoveAt(TransformIndex);
        }
    }
}
void AProcFloor::SpawnNPCs()
{
    FHouseNPCTypeInfo ChosenNPC = GetRandomObjectType();
    if (ChosenNPC.NPCBlueprint != nullptr)
    {
        FActorSpawnParameters SpawnParams;
        // Set any necessary spawn parameters here

        ACharacter* SpawnedNPC = GetWorld()->SpawnActor<ACharacter>(ChosenNPC.NPCBlueprint, GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
        // Additional logic after spawning (if needed)
    }
}



FHouseNPCTypeInfo AProcFloor::GetRandomObjectType()
{
    //todo: weighted influence based on floor type
    if (NPCTypes.Num() > 0)
    {
        int32 RandomIndex = RS.RandRange(0, NPCTypes.Num() - 1);
        return NPCTypes[RandomIndex];
    }

    return struct FHouseNPCTypeInfo();
}

