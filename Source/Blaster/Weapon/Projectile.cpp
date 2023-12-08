#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Limb/Limb.h"
#include "Blaster/Blaster.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"


AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	//we spawn these projectiles on the server, which is replicated down to the clients
	//the server maintains authority over the location of the projectile
	bReplicates = true;
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);	
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block );
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block );
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic , ECollisionResponse::ECR_Block );
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);
	
}


void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}
	
	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}

	if (HasAuthority())
	{//hit events handled only on server
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}
}


void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//This is behaving oddly
void AProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AProjectile, bCharacterWasHit);
}

void AProjectile::ExplodeDamage()
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,//world context object
				Damage,//base damage
				10.f,//minimum damage (at outer radius)
				GetActorLocation(),//origin location
				DamageInnerRadius,//inner radius
				DamageOuterRadius,//outer radius
				1.f, //damage falloff
				UDamageType::StaticClass(),//damage type class
				TArray<AActor*>(),//ignore actors (empty array)
				this,//damage causer
				FiringController //InstigatorController
			);
		}
	}
}


void AProjectile::SpawnTrailSystem()
{
	if (TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),//bone socket attach point
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false//bAutoDestroy
		);
	}
}

//first must b bound to onComponentHit in BeginPlay, doing constructor is too early
void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	bCharacterWasHit = false;
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		//not calling an rpc?
		//BlasterCharacter->MulticastHit();
		bCharacterWasHit = true;
	} 
	else
	{
		ALimb* LimbCharacter = Cast<ALimb>(OtherActor);
		if (LimbCharacter)
		{
			bCharacterWasHit = true;
		}
	}
	
	Multicast_OnHit(bCharacterWasHit);
}

void AProjectile::Multicast_OnHit_Implementation(bool bCharacterHit)
{
	//blood splatter or environmental particle effect
	ImpactParticles = bCharacterHit ? ImpactCharacterParticles : ImpactEnvironmentParticles;
	ImpactSound = bCharacterHit ? ImpactCharacterSound : ImpactEnvironmentSound;
	if (ImpactParticles)
	{ //include gameplaystatics
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{ //#include "Sound/SoundCue.h"
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	Destroy();
}

void AProjectile::StartDestroyTimer()
{
	//dont want to destroy quite on hit
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectile::DestroyTimerFinished,
		DestroyTime
	);
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}

void AProjectile::Destroyed()
{
	Super::Destroyed();
}

void AProjectile::ActivateImpactSoundAndParticles(AActor* OtherActor)
{
	bCharacterWasHit = false;
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		//not calling an rpc?
		//BlasterCharacter->MulticastHit();
		bCharacterWasHit = true;
	}

	ImpactParticles = bCharacterWasHit ? ImpactCharacterParticles : ImpactEnvironmentParticles;
	ImpactSound = bCharacterWasHit ? ImpactCharacterSound : ImpactEnvironmentSound;


	if (ImpactParticles)
	{ //include gameplaystatics
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{ //#include "Sound/SoundCue.h"
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}