
#include "RoamingChaser.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Components/SphereComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationInvokerComponent.h"
#include "NavigationSystem.h"


ARoamingChaser::ARoamingChaser()
{
    // Initialize AI Perception Component
    AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    // Configure AI Perception here as needed

    // Initialize the Navigation Invoker Component
    NavigationInvoker = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavigationInvokerComponent"));

    // Initialize the Damage Sphere Component and attach it to the LeftHandSocket
    DamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DamageSphere"));
    DamageSphere->SetupAttachment(GetMesh(), FName("LeftHandSocket"));
    DamageSphere->OnComponentBeginOverlap.AddDynamic(this, &ARoamingChaser::OnDamageSphereOverlap);

    // Set the initial AI state
    CurrentAIState = EAIState::Idle;
}

void ARoamingChaser::BeginPlay()
{
    Super::BeginPlay();

    // Set up initial walking speed
    GetCharacterMovement()->MaxWalkSpeed = 100.0f;

    // Set up the timer for roaming
    GetWorldTimerManager().SetTimer(MovementTimer, this, &ARoamingChaser::ChooseNewLocation, FMath::RandRange(5.0f, 10.0f), true);

    // Set up the timer for the line of sight check
    GetWorldTimerManager().SetTimer(LineOfSightCheckTimer, this, &ARoamingChaser::CheckPlayerLineOfSight, 1.0f, true);
}

void ARoamingChaser::ChooseNewLocation()
{
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    FNavLocation NewNavLocation;
    if (NavSys && NavSys->GetRandomPointInNavigableRadius(GetActorLocation(), RoamingRadius, NewNavLocation))
    {
        AAIController* AIController = Cast<AAIController>(GetController());
        if (AIController)
        {
            AIController->MoveToLocation(NewNavLocation.Location);
        }
    }
}

void ARoamingChaser::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor->IsA(ABlasterCharacter::StaticClass()))
        {
            DetectedPlayer = Cast<ABlasterCharacter>(Actor);
            if (DetectedPlayer)
            {
                // Stop roaming and start staring at the player
                GetWorldTimerManager().ClearTimer(MovementTimer);
                CurrentAIState = EAIState::Staring;
                break;
            }
        }
    }
}

void ARoamingChaser::CheckPlayerLineOfSight()
{
    if (CurrentAIState != EAIState::Staring || !DetectedPlayer)
        return;

    FVector DirectionToPlayer = DetectedPlayer->GetActorLocation() - GetActorLocation();
    DirectionToPlayer.Z = 0;
    FRotator NewLookAt = FRotationMatrix::MakeFromX(DirectionToPlayer).Rotator();
    SetActorRotation(NewLookAt);

    FVector PlayerForwardVector = DetectedPlayer->GetActorForwardVector();
    FVector ToAICharacter = GetActorLocation() - DetectedPlayer->GetActorLocation();
    ToAICharacter.Normalize();

    float DotProduct = FVector::DotProduct(PlayerForwardVector, ToAICharacter);
    if (DotProduct > 0.0f)
    {
        // Player spotted the AI - transition to chasing
        CurrentAIState = EAIState::Chasing;
        GetCharacterMovement()->MaxWalkSpeed = 500.0f;
        AAIController* AIController = Cast<AAIController>(GetController());
        if (AIController)
        {
            AIController->MoveToActor(DetectedPlayer, 5.0f, true, true, true, 0, true);
        }
    }
}

void ARoamingChaser::OnDamageSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA(ABlasterCharacter::StaticClass()))
    {
        CurrentAIState = EAIState::Laughing;

        // Play the LaughSoundCue
        if (LaughSoundCue)
        {
            UGameplayStatics::PlaySoundAtLocation(this, LaughSoundCue, GetActorLocation());
        }

        // Set timer to return to Idle state after 10 seconds
        GetWorldTimerManager().SetTimer(StateTimer, this, &ARoamingChaser::ReturnToIdle, 10.0f, false);
    }
    else
    {
        CurrentAIState = EAIState::Fleeing;

        // Increase speed for fleeing
        GetCharacterMovement()->MaxWalkSpeed = 600.0f; // Assuming 600 is the fleeing speed

        // Choose a distant location to flee to
        FVector FleeLocation = ChooseFleeLocation();
        AAIController* AIController = Cast<AAIController>(GetController());
        if (AIController)
        {
            AIController->MoveToLocation(FleeLocation);
        }

        // Optionally, set a timer to return to Idle state after reaching the location or after a duration
        // ...
    }
}

FVector ARoamingChaser::ChooseFleeLocation()
{
    // Logic to choose a distant location for fleeing
    // This can be similar to ChooseNewLocation but with a larger radius
   // FVector FleeLocation = ...; // Implement the logic to choose a distant location
    FVector FleeLocation(0,0,0);
    return FleeLocation;
}

void ARoamingChaser::ReturnToIdle()
{
    CurrentAIState = EAIState::Idle;
    GetCharacterMovement()->MaxWalkSpeed = 100.0f; // Reset speed to normal walking speed

    // Resume normal behavior such as roaming
    GetWorldTimerManager().SetTimer(MovementTimer, this, &ARoamingChaser::ChooseNewLocation, FMath::RandRange(5.0f, 10.0f), true);
}
