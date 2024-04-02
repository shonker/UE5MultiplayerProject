// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/ProceduralEnvironments/AProcActor.h"
#include "GameFramework/Actor.h"
#include "ProcFurniture.h"
#include "ProcFloor.generated.h"


USTRUCT(BlueprintType)
struct FCeilingSpawnables
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AProcFurniture> ObjectClass;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Tag;  // "clock", "scary", etc
};

USTRUCT(BlueprintType)
struct FFloorSpawnables
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AProcFurniture> ObjectClass;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Tag;  // "clock", "scary", etc
};

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
	virtual void ProcGen() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ceiling Objects") 
	TArray<FCeilingSpawnables> SpawnableCeilingObjects;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output Locations")
	TArray<FTransform> CeilingObjectTransforms;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Objects") 
	TArray<FCeilingSpawnables> SpawnableFloorObjects;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output Locations")
	TArray<FTransform> FloorObjectTransforms;
	
	UPROPERTY(EditAnywhere, Category = "Procedural Generation")
		TArray<FHouseNPCTypeInfo> NPCTypes; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Output Locations")
		TArray<FTransform> NPCOutputLocs;

	FHouseNPCTypeInfo GetRandomObjectType();

	UPROPERTY(EditAnywhere, Category = "Ceiling Objects")
	bool bIsCeiling = false;

private:
	void SpawnNPCs();
	void GenerateCeilingObjects();
	void GenerateFloorObjects();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};