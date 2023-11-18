#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Limb/Limb.h"
#include "Blaster/Blaster.h"
#include "Net/UnrealNetwork.h"

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
	//CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn , ECollisionResponse::ECR_Block );
	//CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody , ECollisionResponse::ECR_Block );
	 
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
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

//these were moved to Destroyed() from OnHit(), as they needed to be replicated down to clients and Destroyed already IS replicated
//this way we utilize something already replicated, reducing total bandwidth
/*
if (ImpactParticles)
{ //include gameplaystatics
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
}
if (ImpactSound)
{ //#include "Sound/SoundCue.h"
	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
}
*/

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

	Destroy();
}

void AProjectile::Destroyed()
{
	Super::Destroyed();

	if (ImpactParticles)
	{ //include gameplaystatics
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{ //#include "Sound/SoundCue.h"
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}


}
/*

/////////------------------------------------/////////
/////////------------------------------------/////////
			WRITEUP ABOUT REPLICATION
/////////------------------------------------/////////
/////////------------------------------------/////////


To get the Impact particles to properly replicate server -> client:

//////////////header file:

UFUNCTION(NetMulticast, Reliable)
void Multicast_OnHit(bool bCharacterHit);

UPROPERTY(Replicated)
bool bCharacterWasHit;

//////////////.cpp:

//in the begin play, only if HasAuthority() is OnHit bound to the collision event

// When the server calls OnHit, the replicated var, bCharacterWasHit,
// is passed into Multicast_OnHit

//it is only a bool so its not very consequential to bandwidth
//when all clients call the multicast they set the vfx client side


/////////------------------------------------/////////
/////////------------------------------------/////////
					QUESTIONS
/////////------------------------------------/////////
/////////------------------------------------/////////

Why do I not need to override GetLifetimeReplicated props?
Or need to use DOREPLIFETIME()?

--it suddenly stopped compiling until i added them in....
i guess it was necessary but somehow was working for a while withoutem


*/

