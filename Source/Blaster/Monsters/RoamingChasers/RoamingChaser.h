
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"
#include "RoamingChaser.generated.h"

// Define AI states
UENUM(BlueprintType)
enum class EAIState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Roaming UMETA(DisplayName = "Roaming"),
    Staring UMETA(DisplayName = "Staring"),
    Chasing UMETA(DisplayName = "Chasing"),
    Laughing UMETA(DisplayName = "Laughing"),
    Fleeing UMETA(DisplayName = "Fleeing")
    // ... other states as needed
};

UCLASS()
class BLASTER_API ARoamingChaser : public ACharacter
{
    GENERATED_BODY()

public:
    // Constructor
    ARoamingChaser();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // AI Perception Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class   UAIPerceptionComponent* AIPerception;

    // Navigation Invoker Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class   UNavigationInvokerComponent* NavigationInvoker;

    // Damage Sphere Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
        USphereComponent* DamageSphere;

    // AI States
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    EAIState CurrentAIState;

    // Timer handle for movement
    FTimerHandle MovementTimer;

    // Timer handle for line of sight check
    FTimerHandle LineOfSightCheckTimer;

    // Timer handle for state duration (e.g., Laughing state duration)
    FTimerHandle StateTimer;

    // Roaming Radius
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", UIMin = "0.0"))
        float RoamingRadius;

    // Fleeing Speed
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", UIMin = "0.0"))
        float FleeingSpeed;

    // Reference to the detected player character
    class ABlasterCharacter* DetectedPlayer;

    // Sound Cues
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
        USoundCue* LaughSoundCue;

    // Function to choose a new location for roaming
    void ChooseNewLocation();

    // Function called when perception is updated
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Function to check player line of sight
    void CheckPlayerLineOfSight();

    // Function called when the damage sphere overlaps with another actor
    UFUNCTION()
        void OnDamageSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // Function to choose a flee location
    FVector ChooseFleeLocation();

    // Function to return to idle state
    void ReturnToIdle();

    // ... Any other declarations as needed
};
