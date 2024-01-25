
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
    AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    // Configure AI Perception here as needed

    NavigationInvoker = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavigationInvokerComponent"));

    DamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DamageSphere"));
    DamageSphere->SetupAttachment(GetMesh(), FName("RightHandSocket"));
    DamageSphere->OnComponentBeginOverlap.AddDynamic(this, &ARoamingChaser::OnDamageSphereOverlap);

    CurrentAIState = EAIState::Idle;
}

void ARoamingChaser::BeginPlay()
{
    Super::BeginPlay();

    GetCharacterMovement()->MaxWalkSpeed = 100.0f;

    GetWorldTimerManager().SetTimer(MovementTimer, this, &ARoamingChaser::ChooseNewLocation, FMath::RandRange(5.0f, 10.0f), true);
  
    AIPerception->OnPerceptionUpdated.AddDynamic(this, &ARoamingChaser::OnPerceptionUpdated);

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
                UE_LOG(LogTemp, Log, TEXT("i have detected the player"));
                // Stop roaming and start staring at the player
                GetWorldTimerManager().ClearTimer(MovementTimer);
                GetWorldTimerManager().SetTimer(LineOfSightCheckTimer, this, &ARoamingChaser::CheckPlayerLineOfSight, 0.2f, true);
                GetWorldTimerManager().SetTimer(ChaseDurationTimer,
                    this, 
                    &ARoamingChaser::GiveUpOnChasing, 
                    FMath::RandRange(5,12), 
                    false
                );
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
    if (DotProduct > 0.0f && CurrentAIState != EAIState::Chasing)
    {
        CurrentAIState = EAIState::Chasing;
        GetCharacterMovement()->MaxWalkSpeed = 500.0f;
        AAIController* AIController = Cast<AAIController>(GetController());
        if (AIController)
        {
            UE_LOG(LogTemp, Log, TEXT("i have decided the player sees me and am chasing the player"));
            AIController->MoveToActor(DetectedPlayer, 5.0f, true, true, true, 0, true);
        }
    }
    if (CurrentAIState == EAIState::Chasing)
    {
        float DistanceToPlayer = (DetectedPlayer->GetActorLocation() - GetActorLocation()).Size();
        if (DistanceToPlayer <= 600.0f)
        {
            UE_LOG(LogTemp, Log, TEXT("i store player location and will attack in a moment"));
            StoredPlayerLocation = DetectedPlayer->GetActorLocation();
            GetWorldTimerManager().ClearTimer(LineOfSightCheckTimer);
            GetWorldTimerManager().SetTimer(StateTimer, this, &ARoamingChaser::CommenceAttack, 0.2f, false);
        }
    }
}

void ARoamingChaser::CommenceAttack()
{
    UE_LOG(LogTemp, Log, TEXT("i attack"));
    GetCharacterMovement()->MaxWalkSpeed = 650.0f;
    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController)
    {
        AIController->MoveToLocation(StoredPlayerLocation);
    }
    bInitiateAttack = true; // This will be used to signal the animation blueprint
}

void ARoamingChaser::OnDamageSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this)
    {
        OverlappedActors.AddUnique(OtherActor);
    }
}

void ARoamingChaser::CheckAttackResult()
{
    bInitiateAttack = false;
    bool bHitBlasterCharacter = false;
    for (AActor* Actor : OverlappedActors)
    {
        if (Actor && Actor->IsA(ABlasterCharacter::StaticClass()))
        {
            UE_LOG(LogTemp, Log, TEXT("i hit the blaster character"));
            bHitBlasterCharacter = true;
            break;
        }
    }
    CurrentAIState = bHitBlasterCharacter ? EAIState::Laughing : EAIState::Fleeing;
    OverlappedActors.Empty();

    if (CurrentAIState == EAIState::Laughing)
    {
        if (LaughSoundCue)
        {
            UGameplayStatics::PlaySoundAtLocation(this, LaughSoundCue, GetActorLocation());
        }
        UE_LOG(LogTemp, Log, TEXT("i laugh"));
    }
    else
    {
        GetCharacterMovement()->MaxWalkSpeed = 600.0f;

        FVector FleeLocation = ChooseFleeLocation();
        AAIController* AIController = Cast<AAIController>(GetController());
        if (AIController)
        {
            UE_LOG(LogTemp, Log, TEXT("i cry and flee"));
            AIController->MoveToLocation(FleeLocation);
        }
    }
    GetWorldTimerManager().SetTimer(StateTimer, this, &ARoamingChaser::ReturnToIdle, 10.0f, false);
}

FVector ARoamingChaser::ChooseFleeLocation()
{
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    FNavLocation NewNavLocation;
    if (NavSys && NavSys->GetRandomPointInNavigableRadius(GetActorLocation(), RoamingRadius*15, NewNavLocation))
    {
        return NewNavLocation;
    }
   
    FVector FleeLocation(0,0,0);
    return FleeLocation;
}

void ARoamingChaser::ReturnToIdle()
{
    UE_LOG(LogTemp, Log, TEXT(" i return to idle"));
    CurrentAIState = EAIState::Idle;
    GetCharacterMovement()->MaxWalkSpeed = 100.0f;
    GetWorldTimerManager().ClearTimer(LineOfSightCheckTimer);
    GetWorldTimerManager().SetTimer(MovementTimer, this, &ARoamingChaser::ChooseNewLocation, FMath::RandRange(5.0f, 10.0f), true);
}
