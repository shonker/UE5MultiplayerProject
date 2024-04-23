// Fill out your copyright notice in the Description page of Project Settings.


#include "TubeSlide.h"
#include "Components/SphereComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"



ATubeSlide::ATubeSlide()
{
	PrimaryActorTick.bCanEverTick = true;

	WormMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Worm Mesh"));
	SetRootComponent(WormMesh);
	WormMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

	TubeSlide = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tube Slide"));
	TubeSlide->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	TubeSlide->SetupAttachment(RootComponent);

	DamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Damage Sphere"));
	DamageSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DamageSphere->SetupAttachment(WormMesh, FName("DamageSphereSocket"));

	AttackSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Attack Sphere"));
	AttackSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackSphere->SetupAttachment(RootComponent);

	AttentionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Attention Sphere"));
	AttentionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttentionSphere->SetupAttachment(RootComponent);
	bReplicates = true;
}

void ATubeSlide::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		DamageSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		DamageSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
		DamageSphere->OnComponentBeginOverlap.AddDynamic(this, &ATubeSlide::OnDamageSphereOverlap);
		
		AttentionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AttentionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
		AttentionSphere->OnComponentBeginOverlap.AddDynamic(this, &ATubeSlide::OnAttentionSphereOverlap);
		AttentionSphere->OnComponentEndOverlap.AddDynamic(this, &ATubeSlide::OnAttentionSphereEndOverlap);

		AttackSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AttackSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
		AttackSphere->OnComponentBeginOverlap.AddDynamic(this, &ATubeSlide::OnAttackSphereOverlap);
	}
	AttackTarget = GetActorLocation();
}

void ATubeSlide::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATubeSlide, bAttentive);
	DOREPLIFETIME(ATubeSlide, bAttacking);
	DOREPLIFETIME(ATubeSlide, AttackTarget);
}

void ATubeSlide::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bAttacking)
	{
		if (TargetCharacter != nullptr) AttackTarget = TargetCharacter->GetActorLocation();
	}

}

void ATubeSlide::OnAttentionSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FString ComponentName = OtherComp->GetName();
	if (ComponentName == FString("CollisionCylinder"))
	{
		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
		if (BlasterCharacter)
		{
			CharactersInAttentionRadius++;
			//	UE_LOG(LogTemp, Log, TEXT("overlap detected in tube worm on server"));
			bAttentive = true;
		}
	}
}

void ATubeSlide::OnAttentionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	FString ComponentName = OtherComp->GetName();
	if (ComponentName == FString("CollisionCylinder"))
	{
		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
		if (BlasterCharacter)
		{
			CharactersInAttentionRadius--;
			//	UE_LOG(LogTemp, Log, TEXT("overlap END detected in tube worm on server"));

			if (CharactersInAttentionRadius == 0)
			{
				bAttentive = false;
				bAttacking = false;
			}
		}
	}
}

void ATubeSlide::OnAttackSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FString ComponentName = OtherComp->GetName();
	if (ComponentName == FString("CollisionCylinder"))
	{
		TargetCharacter = Cast<ABlasterCharacter>(OtherActor);
		AttackTarget = OtherActor->GetActorLocation();
		if (bAttentive)
		{
			bAttacking = true;
			GetWorldTimerManager().SetTimer(
				AggroTimer,
				this,
				&ATubeSlide::AggroTimerFinished,
				FMath::RandRange(1, 2)
			);
		}
	}
}

void ATubeSlide::AggroTimerFinished()
{
	if (FMath::RandBool())
	{
		if (TargetCharacter)
		{
			AttackTarget = TargetCharacter->GetActorLocation();
		}

		bAttacking = true;
		GetWorldTimerManager().SetTimer(
			AggroTimer,
			this,
			&ATubeSlide::AggroTimerFinished,
			FMath::RandRange(3, 10)
		);
	}
	else
	{
		bAttacking = false;
	}
}

void ATubeSlide::OnDamageSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FString ComponentName = OtherComp->GetName();
	if (ComponentName == FString("CollisionCylinder"))
	{
		ABlasterCharacter* Char = Cast<ABlasterCharacter>(OtherActor);
		if (Char && HasAuthority())
		{
			UE_LOG(LogTemp, Log, TEXT("damage player?"));

			UGameplayStatics::ApplyDamage(
				Char,
				25.f,
				Char->GetController(),
				this,
				UDamageType::StaticClass()
			);
		}
	}

	////only called on server
	//void ABlasterCharacter::ReceiveDamage(
	//	AActor * DamagedActor,
	//	float Damage,
	//	const UDamageType * DamageType,
	//	AController * InstigatorController,
	//	AActor * DamageCauser)
}

