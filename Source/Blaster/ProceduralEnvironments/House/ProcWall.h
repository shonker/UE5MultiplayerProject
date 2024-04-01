#pragma once

#include "CoreMinimal.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Blaster/ProceduralEnvironments/AProcActor.h"
#include "ProcWall.generated.h"

USTRUCT(BlueprintType)
struct FNarrowWallSpawnables
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AActor> ObjectClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> Tag; //"clock", "poop", etc
};

USTRUCT(BlueprintType)
struct FWideWallSpawnables
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AActor> ObjectClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> Tag;  // "clock", "scary", etc
};

UCLASS()
class BLASTER_API AProcWall : public AAProcActor
{
    GENERATED_BODY()

public:
    AProcWall();
    virtual void ProcGen() override;
    void InitializeRandomSpawning();
    void SpawnWideObject();

protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Objects") 
    TArray<FNarrowWallSpawnables> SpawnableNarrowObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Objects")
    TArray<FWideWallSpawnables> SpawnableWideObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output Locations")
    TArray<FTransform> WideObjectTransforms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output Locations")
    TArray<FTransform> NarrowObjectTransforms;

    void SpawnNarrowObjects();
    int32 CalculateObjectsToSpawn(int32 NumTransforms) const;
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Settings")
        UStaticMeshComponent* WallMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Settings")
       class UGeometryCollectionComponent* GeometryCollectionComponent;

    //dont forget to generate onhit collision events
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Settings")
        bool bBreakableOnImpact;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Settings")
        bool bBreakable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Settings")
        UParticleSystem* BreakParticle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Settings")
        UParticleSystem* DamagedParticle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Settings")
        USoundCue* BreakSoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Settings")
        float WallHealthMax = 100.f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Settings")
        float WallHealthMin = 50.f;

    float WallHealth;

    UPROPERTY(ReplicatedUsing = OnRep_bIsBroken)
        bool bIsBroken = false;

    UFUNCTION()
        void OnRep_bIsBroken();

    UFUNCTION()
        void OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    UFUNCTION()
        void BreakWindow();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wall Settings")
        class USceneComponent* DefaultRoot;
    
    UFUNCTION()
        void TakeWallDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

    void DestroyActor();

private:
    FTimerHandle DestructionTimer;

  
public:
    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
};
