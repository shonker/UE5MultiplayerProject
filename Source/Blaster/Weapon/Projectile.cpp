// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Blaster.h"

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
	{
		//hit events handled only on server
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}
}

//first must b bound to onComponentHit in BeginPlay, doing constructor is too early
void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//these were moved to Destroyed(), as they needed to be replicated down to clients and Destroyed already IS replicated
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
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->MulticastHit();
	}

	Destroy();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
