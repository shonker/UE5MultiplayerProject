// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Casing.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/BlasterComponents/CombatComponent.h"

// Sets default values
AWeapon::AWeapon()
{
	
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	//important for server/clientcomms?
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	if (WeaponMesh)
	{
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (WeaponMesh->IsSimulatingPhysics()) WeaponMesh->SetSimulatePhysics(false);
	}

	AreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaBox"));
	if (AreaBox)
	{
		AreaBox->SetupAttachment(RootComponent);
		AreaBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

	}
	

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	if (PickupWidget) PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		AreaBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaBox->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}

	if (WeaponMesh)
	{
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (WeaponMesh->IsSimulatingPhysics()) WeaponMesh->SetSimulatePhysics(false);
	}

	if (AreaBox)
	{
		AreaBox->SetupAttachment(RootComponent);
		AreaBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
		AreaBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	}

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherComp->GetName() == FString("InteractSphere"))
	{
		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherComp->GetOwner());
		if (BlasterCharacter)
		{
			BlasterCharacter->SetOverlappingWeapon(this);
		}
	}
};

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex)
{
	if (OtherComp->GetName() == FString("InteractSphere"))
	{
		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherComp->GetOwner());
		if (BlasterCharacter)
		{
			BlasterCharacter->SetOverlappingWeapon(nullptr);
		}
	}
}

void AWeapon::SetHUDAmmo()
{
	BlasterOwnerCharacter =
		BlasterOwnerCharacter == nullptr ?
		Cast<ABlasterCharacter>(GetOwner()) :
		BlasterOwnerCharacter;
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController =
			BlasterOwnerController == nullptr ?
			Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) :
			BlasterOwnerController;
		if (BlasterOwnerController)
		{
			BlasterOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::OnRep_Ammo()
{
	BlasterOwnerCharacter =
		BlasterOwnerCharacter == nullptr ?
		Cast<ABlasterCharacter>(GetOwner()) :
		BlasterOwnerCharacter;
	if (BlasterOwnerCharacter
		&& BlasterOwnerCharacter->GetCombat()
		&& IsFull()) 
	{
		BlasterOwnerCharacter->GetCombat()->JumpToShotgunEnd();
	}
	SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{//we do this to initialize the ammo when gun is picked up
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		BlasterOwnerCharacter = nullptr;
		BlasterOwnerController = nullptr;
	}
	{
		SetHUDAmmo();
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnSetWeaponState(State);
}

//only one replicated var-> weap state
void AWeapon::OnRep_WeaponState()
{
	OnSetWeaponState(WeaponState);
}

void AWeapon::OnSetWeaponState(EWeaponState State)
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_PickedUp:
		ShowPickupWidget(false);
		WeaponMesh->SetVisibility(true);
		if (WeaponMesh->IsSimulatingPhysics()) WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AreaBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(false);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			WeaponMesh->WakeAllRigidBodies();
		}
		if (HasAuthority())
		{
			SetWeaponState(EWeaponState::EWS_Equipped);
		}
		break;
	
	case EWeaponState::EWS_Equipped:
		WeaponMesh->SetVisibility(true);
	break;

	case EWeaponState::EWS_Dropped:
		AreaBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
		AreaBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

		WeaponMesh->SetVisibility(true);
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
		break;

	case EWeaponState::EWS_Stored:
		WeaponMesh->SetVisibility(false);
		break;
	}
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}

	if (CasingClass)
	{//#include "Engine/SkeletalMeshSocket.h"
		const USkeletalMeshSocket* AmmoEjectSocket = GetWeaponMesh()->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(GetWeaponMesh());

			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
				);
			}
		}
	}
	SpendRound();
}

void AWeapon::Dropped()
{
	UE_LOG(LogTemp, Log, TEXT("i am the dropped weapon"));
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}

bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}

bool AWeapon::IsFull()
{
	return Ammo == MagCapacity;
}