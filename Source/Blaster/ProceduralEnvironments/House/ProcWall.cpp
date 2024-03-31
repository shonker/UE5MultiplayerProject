#include "ProcWall.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Limb/Limb.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

#include "Components/StaticMeshComponent.h"

AProcWall::AProcWall()
{
    PrimaryActorTick.bCanEverTick = false;
    DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
    RootComponent = DefaultRoot;
    WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
    WallMesh->SetupAttachment(RootComponent);
    WallMesh->SetCollisionProfileName(TEXT("BlockAll"));    
}

void AProcWall::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority() && bBreakable)
    {
        WallHealth = RS.RandRange(WallHealthMin, WallHealthMax);
        OnTakeAnyDamage.AddDynamic(this, &AProcWall::TakeWallDamage);
        if (bBreakableOnImpact)
        {
            WallMesh->OnComponentHit.AddDynamic(this, &AProcWall::OnComponentHit);
        }
    }
}

void AProcWall::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AProcWall, bIsBroken);
}


void AProcWall::OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    float ImpulseLength = NormalImpulse.Size();
    UE_LOG(LogTemp, Warning, TEXT("Impulse Length: %f"), ImpulseLength);

    if (bIsBroken) return;
    UE_LOG(LogTemp, Log, TEXT("been hit"));
    ABlasterCharacter* BlasterChar = Cast<ABlasterCharacter>(OtherActor);
    if (BlasterChar)
    {
        UE_LOG(LogTemp, Log, TEXT("by a blasterchar"));
        return;
    }

    if (NormalImpulse.Size() > 9000.f)
    {
        UE_LOG(LogTemp, Log, TEXT("force > 9k"));
        BreakWindow();
        return;
    }

    ALimb* Limb = Cast<ALimb>(OtherActor);
    if (!Limb && NormalImpulse.Size() > 500.f)
    {
        UE_LOG(LogTemp, Log, TEXT("by not limb"));
        BreakWindow();
    }
}

void AProcWall::BreakWindow()
{
    bIsBroken = true;
    if (BreakSoundCue)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BreakSoundCue, GetActorLocation());
    }
    if (BreakParticle)
    {
        FVector ParticleLoc = GetActorLocation() + FVector(0, 0, 300.f);
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BreakParticle, ParticleLoc, GetActorRotation(), FVector(1.0f));
    }
    WallMesh->SetVisibility(false);
    WallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WallMesh->SetSimulatePhysics(false);
    WallMesh->Deactivate();
    DefaultRoot->Activate(true);
    GetWorld()->GetTimerManager().SetTimer(DestructionTimer, this, &AProcWall::DestroyActor, 10.0f, false);
}

void AProcWall::OnRep_bIsBroken()
{
    if (!bIsBroken) return;
    if (BreakSoundCue)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BreakSoundCue, GetActorLocation());
    }
    if (BreakParticle)
    {
        FVector ParticleLoc = GetActorLocation() + FVector(0, 0, 300.f);
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BreakParticle, ParticleLoc, GetActorRotation(), FVector(1.0f));
    }
    WallMesh->SetVisibility(false);
    WallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WallMesh->SetSimulatePhysics(false);
    WallMesh->Deactivate();
    DefaultRoot->Activate(true);
    GetWorld()->GetTimerManager().SetTimer(DestructionTimer, this, &AProcWall::DestroyActor, 10.0f, false);
}

void AProcWall::TakeWallDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    UE_LOG(LogTemp, Log, TEXT("my leg: %f"), Damage);
    if (bIsBroken) return;

    WallHealth -= Damage;
    if (WallHealth <= 0)
    {
        BreakWindow();
    }
    if (DamagedParticle)
    {
        FVector ParticleLoc = GetActorLocation() + FVector(0, 0, 300.f);
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DamagedParticle, ParticleLoc, GetActorRotation(), FVector(1.0f));
    }
}

void AProcWall::DestroyActor()
{
    Destroy();
}

