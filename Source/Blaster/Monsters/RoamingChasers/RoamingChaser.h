
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
    Fleeing UMETA(DisplayName = "Fleeing"),
    Attacking UMETA(DisplayName = "Attacking")
};

UCLASS()
class BLASTER_API ARoamingChaser : public ACharacter
{
    GENERATED_BODY()

public:
    ARoamingChaser();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable)
        void CheckAttackResult();
    
    UPROPERTY(BlueprintReadOnly, Replicated)
    bool bInitiateAttack = false;
    UPROPERTY(BlueprintReadWrite, Replicated)
    bool bLaughing = false;
    UPROPERTY(Replicated)
    bool bHitBlasterCharacter = false;

    UFUNCTION(Server, Reliable)
    void ServerSetAIState(EAIState State);
    
    class AAIController* AIController;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAYDENS Custom Setup")
    class   UAIPerceptionComponent* AIPerception;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAYDENS Custom Setup")
    class   UNavigationInvokerComponent* NavigationInvoker;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAYDENS Custom Setup")
        USphereComponent* DamageSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CAYDENS Custom Setup")
        class USphereComponent* StabSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "CAYDENS Custom Setup")
    EAIState CurrentAIState;

    FTimerHandle MovementTimer;

    FTimerHandle LineOfSightCheckTimer; 

    FTimerHandle ChaseDurationTimer;

    FTimerHandle IdleReturnTimer;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CAYDENS Custom Setup", meta = (ClampMin = "0.0", UIMin = "0.0"))
        float RoamingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CAYDENS Custom Setup", meta = (ClampMin = "0.0", UIMin = "0.0"))
        float FleeingSpeed;

    class ABlasterCharacter* DetectedPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CAYDENS Custom Setup")
        USoundCue* LaughSoundCue;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CAYDENS Custom Setup")
        USoundCue* CrySoundCue;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CAYDENS Custom Setup")
        USoundCue* StabSoundCue;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CAYDENS Custom Setup")
        UParticleSystem* StabParticles;

    void ChooseNewLocation();

    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    void CheckPlayerLineOfSight();

    void CommenceAttack();

    void GiveUpOnChasing();


    UFUNCTION()
    void OnDamageSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    UFUNCTION()
    void OnAttackSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    FVector ChooseFleeLocation();

    void ReturnToIdle();

private:
    FVector StoredPlayerLocation;
};
