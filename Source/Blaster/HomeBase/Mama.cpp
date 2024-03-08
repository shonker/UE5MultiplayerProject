// Fill out your copyright notice in the Description page of Project Settings.

#include "Mama.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Net/UnrealNetwork.h"
#include "HomeBase.h"

// Constructor
AMama::AMama()
{
	PrimaryActorTick.bCanEverTick = true;

	MamaHead = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MamaHead"));
	InvisBarrier = CreateDefaultSubobject<UCapsuleComponent>(TEXT("InvisBarrier"));
	Consumer = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Consumer"));

	MamaHead->SetupAttachment(RootComponent);
	MamaHead->SetCollisionEnabled(ECollisionEnabled::NoCollision); 

	InvisBarrier->SetupAttachment(MamaHead);
	InvisBarrier->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly); 
	
	Consumer->SetupAttachment(MamaHead);
	Consumer->SetCollisionEnabled(ECollisionEnabled::NoCollision); 


	SetReplicates(true);
}

void AMama::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMama, State);
}

void AMama::BeginPlay()
{
	Super::BeginPlay();
	MamaHead->SetVisibility(true);
	InvisBarrier->SetVisibility(false);
	Consumer->SetVisibility(false);
	if (HasAuthority())
	{
		Consumer->OnComponentBeginOverlap.AddDynamic(this, &AMama::HandleCollision);
		//GetWorld()->GetTimerManager().SetTimer(StateTimerHandle, this, &AMama::BeginActivatingState, 1.0f, false);
	}
}

void AMama::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMama::BeginActivatingState()
{
	ChangeState(EState::ACTIVATING);
}

void AMama::BeginConsumingState()
{
	ChangeState(EState::CONSUMING);
}

void AMama::EvaluateConsumedMaterials()
{
	if (State == EState::SATED) return;
	ChangeState(EState::STARVED);
}

void AMama::OnRep_State()
{
	ChangeState(State);
}

void AMama::ChangeState(EState NewState)
{
	State = NewState;
	
	switch (State)
	{
	case EState::IDLE:
		MamaHead->SetVisibility(false, true);
		MamaHead->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		InvisBarrier->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		Consumer->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	break;

	case EState::ACTIVATING:
		MamaHead->SetVisibility(true, true);

		if (LoopingHumSFX)
		{
			UGameplayStatics::SpawnSoundAttached(LoopingHumSFX, MamaHead);
		}
	break;

	case EState::CONSUMING:
		InvisBarrier->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		if (HasAuthority())
		{
			Consumer->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			GetWorld()->GetTimerManager().SetTimer(StateTimerHandle, this, &AMama::EvaluateConsumedMaterials, 2.0f, false);
		}
	break;

	case EState::SATED:
		InvisBarrier->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	break;

	case EState::STARVED:
		if (InvisBarrier) InvisBarrier->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		if (HomeBase) HomeBase->MamaStarved();
		break;
	default:

		break;
	}
}


void AMama::HandleCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	UE_LOG(LogTemp, Log, TEXT("col detected"));
	AWeapon* CollidingWeapon = Cast<AWeapon>(OtherActor);
	if (CollidingWeapon)
	{
		int CurseValue = CollidingWeapon->GetCurseValue();
		TotalCurseTally += CurseValue;

		Multi_PlayEffectsForWeapon(GetActorLocation());

		CollidingWeapon->Destroy();
	}

	FString ComponentName = OtherComp->GetName();
	
	if (ComponentName == FString("CollisionCylinder"))
	{
		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
		if (BlasterCharacter)
		{
			TotalCurseTally += 50;
			Multi_PlayEffectsForCharacter(GetActorLocation());
			BlasterCharacter->Elim();
		}
	}

	if (TotalCurseTally >= TargetCurseTally)
	{
		ChangeState(EState::SATED);
	}
}

void AMama::Multi_PlayEffectsForWeapon_Implementation(const FVector& Location)
{
	if (ItemConsumedSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ItemConsumedSFX, Location);
	}
	if (ItemConsumedVFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ItemConsumedVFX, Location);
	}
}

void AMama::Multi_PlayEffectsForCharacter_Implementation(const FVector& Location)
{
	if (CharConsumedSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CharConsumedSFX, Location);
	}
	if (CharConsumedVFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), CharConsumedVFX, Location);
	}
}

void AMama::SetHomeBase(AHomeBase* HB)
{
	HomeBase = HB;
}