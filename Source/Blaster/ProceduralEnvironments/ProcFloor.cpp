// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcFloor.h"
#include "GameFramework/Character.h"

// Sets default values
AProcFloor::AProcFloor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AProcFloor::BeginPlay()
{
	Super::BeginPlay();
   // if (FMath::RandRange(0,10) == 1) SpawnNPCs();
}

// Called every frame
void AProcFloor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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

