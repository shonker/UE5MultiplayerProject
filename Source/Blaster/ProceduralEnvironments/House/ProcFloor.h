// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/ProceduralEnvironments/AProcActor.h"
#include "GameFramework/Actor.h"
#include "ProcFloor.generated.h"

USTRUCT(BlueprintType)
struct FHouseNPCTypeInfo
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<ACharacter> NPCBlueprint;
};


UCLASS()
class BLASTER_API AProcFloor : public AAProcActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProcFloor();

	UPROPERTY(EditAnywhere, Category = "Procedural Generation")
		TArray<FHouseNPCTypeInfo> NPCTypes; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Output Locations")
		TArray<FTransform> NPCOutputLocs;

	FHouseNPCTypeInfo GetRandomObjectType();

private:
	void SpawnNPCs();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};