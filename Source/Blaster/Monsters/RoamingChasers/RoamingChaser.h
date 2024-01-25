
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"
#include "RoamingChaser.generated.h"

UENUM(BlueprintType)
enum class EAIState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Roaming UMETA(DisplayName = "Roaming"),
    Staring UMETA(DisplayName = "Staring"),
    Chasing UMETA(DisplayName = "Chasing"),
    Laughing UMETA(DisplayName = "Laughing"),
    Fleeing UMETA(DisplayName = "Fleeing")
};

UCLASS()
class BLASTER_API ARoamingChaser : public ACharacter
{
    GENERATED_BODY()

public:
    ARoamingChaser();

    UFUNCTION(BlueprintCallable)
        void CheckAttackResult();
    
    UPROPERTY(BlueprintReadWrite)
    bool bInitiateAttack = false;
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class   UAIPerceptionComponent* AIPerception;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class   UNavigationInvokerComponent* NavigationInvoker;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
        USphereComponent* DamageSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    EAIState CurrentAIState;

    FTimerHandle MovementTimer;

    FTimerHandle LineOfSightCheckTimer; 

    FTimerHandle ChaseDurationTimer;

    FTimerHandle StateTimer;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", UIMin = "0.0"))
        float RoamingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", UIMin = "0.0"))
        float FleeingSpeed;

    class ABlasterCharacter* DetectedPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
        USoundCue* LaughSoundCue;

    void ChooseNewLocation();

    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    void CheckPlayerLineOfSight();

    void CommenceAttack();

    void GiveUpOnChasing();


    /*
        state functions
    */

    void Idle();

    UFUNCTION()
        void OnDamageSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    FVector ChooseFleeLocation();

    void ReturnToIdle();

private:
    FVector StoredPlayerLocation;

    TArray<AActor*> OverlappedActors;
};
