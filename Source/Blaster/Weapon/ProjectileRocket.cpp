// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"


AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority())
	{//hit events handled only on server
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}

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
	
	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,//usoundbase
			GetRootComponent(),//scene componentto attach to
			FName(),//attch ptname
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,//volume
			1.f,//pitch
			0.f,//start time
			LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr, //c style cast of a nullptr to a usc so we can use the final arg here
			false
		);
	}

}

void AProjectileRocket::DestroyTimerFinished()
{
	Destroy();
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
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
				200.f,//inner radius
				500.f,//outer radius
				1.f, //damage falloff
				UDamageType::StaticClass(),//damage type class
				TArray<AActor*>(),//ignore actors (empty array)
				this,//damage causer
				FiringController //InstigatorController
			);
		}
	}
	//dont want to destroy quite on hit
	//Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);

	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectileRocket::DestroyTimerFinished,
		DestroyTime
	);

	ActivateImpactSoundAndParticles(OtherActor);

	if (RocketMesh)
	{
		RocketMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
	{
		TrailSystemComponent->GetSystemInstanceController()->Deactivate();
	}

	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}
}

void AProjectileRocket::ActivateImpactSoundAndParticles(AActor* OtherActor)
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

void AProjectileRocket::Destroyed() {

}

