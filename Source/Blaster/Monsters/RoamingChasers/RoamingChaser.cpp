
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
    PrimaryActorTick.bCanEverTick = false;

    AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    // Configure AI Perception here as needed

    NavigationInvoker = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavigationInvokerComponent"));

    DamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DamageSphere"));
    DamageSphere->SetupAttachment(GetMesh(), FName("RightHandSocket"));
    DamageSphere->OnComponentBeginOverlap.AddDynamic(this, &ARoamingChaser::OnDamageSphereOverlap);

    StabSphere = CreateDefaultSubobject<USphereComponent>(TEXT("StabSphere"));
    StabSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    StabSphere->SetupAttachment(RootComponent);
    bReplicates = true;

    CurrentAIState = EAIState::Idle;
}

void ARoamingChaser::BeginPlay()
{
    Super::BeginPlay();

    GetCharacterMovement()->MaxWalkSpeed = 100.0f;

    GetWorldTimerManager().SetTimer(MovementTimer, this, &ARoamingChaser::ChooseNewLocation, FMath::RandRange(5.0f, 10.0f), true);
  
    if (AIPerception) AIPerception->OnPerceptionUpdated.AddDynamic(this, &ARoamingChaser::OnPerceptionUpdated);
    
    if (StabSphere)
    {
        StabSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        StabSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
        StabSphere->OnComponentBeginOverlap.AddDynamic(this, &ARoamingChaser::OnAttackSphereOverlap);
    }
}

void ARoamingChaser::ChooseNewLocation()
{
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    FNavLocation NewNavLocation;
    AIController = AIController ? AIController : Cast<AAIController>(GetController());

    if (NavSys && NavSys->GetRandomPointInNavigableRadius(GetActorLocation(), RoamingRadius, NewNavLocation))
    {
        if (AIController)
        {
            AIController->MoveToLocation(NewNavLocation.Location);
        }
    }
}

void ARoamingChaser::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    if (CurrentAIState != EAIState::Idle) return;
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor->IsA(ABlasterCharacter::StaticClass()))
        {
            DetectedPlayer = Cast<ABlasterCharacter>(Actor);
            if (DetectedPlayer)
            {
                AIController = AIController ? AIController : Cast<AAIController>(GetController());
                if (AIController)  AIController->MoveToLocation(GetActorLocation());

                UE_LOG(LogTemp, Log, TEXT("i have detected the player"));
                GetWorldTimerManager().ClearTimer(MovementTimer);

                GetWorldTimerManager().SetTimer(IdleReturnTimer, this, &ARoamingChaser::ReturnToIdle, 5.0f, false);
                GetWorldTimerManager().SetTimer(LineOfSightCheckTimer, this, &ARoamingChaser::CheckPlayerLineOfSight, 0.1f, true);

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
        bInitiateAttack = false;
        GetWorldTimerManager().ClearTimer(IdleReturnTimer);
        CurrentAIState = EAIState::Chasing;
        GetCharacterMovement()->MaxWalkSpeed = 500.0f;
        AIController = AIController ? AIController : Cast<AAIController>(GetController());
        if (AIController)
        {
            UE_LOG(LogTemp, Log, TEXT("i have decided the player sees me and am chasing the player"));
            AIController->MoveToActor(DetectedPlayer, 5.f, true, true, true, 0, true);
        }
        GetWorldTimerManager().SetTimer(ChaseDurationTimer,
            this,
            &ARoamingChaser::GiveUpOnChasing,
            FMath::RandRange(5, 12),
            false
        );
        GetWorldTimerManager().ClearTimer(LineOfSightCheckTimer);
    }  
}


void ARoamingChaser::OnAttackSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (CurrentAIState != EAIState::Chasing) return;
    if (!OtherActor || !OtherActor->IsA(ABlasterCharacter::StaticClass())) return;
    FString ComponentName = OtherComp->GetName();
    if (ComponentName == FString("CollisionCylinder"))
    {
        CurrentAIState = EAIState::Attacking;
        CommenceAttack();
    }
}


void ARoamingChaser::CommenceAttack()
{
    bHitBlasterCharacter = false;
    
    UE_LOG(LogTemp, Log, TEXT("bInitiateAttack = true"));
    GetCharacterMovement()->MaxWalkSpeed = 650.0f;
    bInitiateAttack = true; // This will be used to signal the animation blueprint
}

void ARoamingChaser::GiveUpOnChasing()
{
    bInitiateAttack = false;

    if (CrySoundCue)
    {
        UGameplayStatics::PlaySoundAtLocation(this, CrySoundCue, GetActorLocation());
    }

    GetCharacterMovement()->MaxWalkSpeed = 600.0f;

    FVector FleeLocation = ChooseFleeLocation();
    
    AIController = AIController ? AIController : Cast<AAIController>(GetController());
    if (AIController)
    {
        UE_LOG(LogTemp, Log, TEXT("i cry and flee"));
        AIController->MoveToLocation(FleeLocation);
    }

    GetWorldTimerManager().SetTimer(IdleReturnTimer, this, &ARoamingChaser::ReturnToIdle, 5.0f, false);
    GetWorldTimerManager().ClearTimer(ChaseDurationTimer);
}

void ARoamingChaser::OnDamageSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (CurrentAIState != EAIState::Attacking) return;
    if (bHitBlasterCharacter == true) return;

    if (OtherActor && OtherActor->IsA(ABlasterCharacter::StaticClass()))
    {
        bHitBlasterCharacter = true;
        ABlasterCharacter* HitChar = Cast<ABlasterCharacter>(OtherActor);

        if (StabParticles)
        {
            USkeletalMeshComponent* MeshComponent = HitChar->GetMesh();
            if (MeshComponent)
            {
                FName SocketName("spine_003Socket");

                UParticleSystemComponent* ParticleComponent = UGameplayStatics::SpawnEmitterAttached(
                    StabParticles, // The particle system you want to spawn
                    MeshComponent,        // The mesh to which you want to attach the particle system
                    SocketName,     // Replace with the socket name or empty string to attach to the mesh directly
                    FVector(0, 0, 0), // Relative location (offset) of the particle system
                    FRotator(0, 0, 0), // Relative rotation of the particle system
                    EAttachLocation::SnapToTarget, // Attachment rule
                    true // Auto destroy
                );
            }
        }  

        if (HasAuthority() && AIController)
        {
            UGameplayStatics::ApplyDamage(
                HitChar,
                24.f,
                AIController,
                this,
                UDamageType::StaticClass()
            );
        }

        if (StabSoundCue)
        {
            UGameplayStatics::PlaySoundAtLocation(this, StabSoundCue, GetActorLocation());
        }
    }
}

void ARoamingChaser::CheckAttackResult()
{  
    if (bHitBlasterCharacter == true)
    {
        bHitBlasterCharacter = false;
        bInitiateAttack = false;
        bLaughing = true;

        UE_LOG(LogTemp, Log, TEXT("i hit the blaster character"));
        CurrentAIState = EAIState::Laughing;
        GetWorldTimerManager().ClearTimer(ChaseDurationTimer);  

        if (LaughSoundCue)
        {
            UGameplayStatics::PlaySoundAtLocation(this, LaughSoundCue, GetActorLocation());
        }
        GetWorldTimerManager().SetTimer(IdleReturnTimer, this, &ARoamingChaser::ReturnToIdle, 5.0f, false);
        return;
    }

    AIController = AIController ? AIController : Cast<AAIController>(GetController());
    if (AIController && DetectedPlayer)
    {
        UE_LOG(LogTemp, Log, TEXT("i missed an attack and am retargeting"));
        AIController->MoveToActor(DetectedPlayer, 5.f, true, true, true, 0, true);
    }
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

    bLaughing = false;
    bInitiateAttack = false;
    
    ChooseNewLocation();
    GetCharacterMovement()->MaxWalkSpeed = 100.0f;

    GetWorldTimerManager().ClearTimer(LineOfSightCheckTimer);
    GetWorldTimerManager().SetTimer(MovementTimer, this, &ARoamingChaser::ChooseNewLocation, FMath::RandRange(5.0f, 10.0f), true);
}
