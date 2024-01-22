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

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	AreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaBox"));
	AreaBox->SetupAttachment(RootComponent);

	//in mp its imp for things like this do be done on server side
	//such as detect overlap
	AreaBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//this will be enabled on the server in begin play
	AreaBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
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
	//here we check if we are the server and enable collision for the weapons
	if (HasAuthority())
	{
		AreaBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

		//binding overlap function on server side
		//if you peek OnCompBegOverlap, you see FCompBegOverSignature 
		//peek it to find they dec a dyn multi sparse delegate w/ six params
		//that is where the func def args are found for the func
		//here we bind our funcs to these events
		AreaBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaBox->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}
	SetWeaponState(EWeaponState::EWS_Dropped);
	AreaBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

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
	//check includes to see where we included blaster char
	if (OtherComp->GetName() == FString("InteractSphere"))
	{
		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherComp->GetOwner());
		//ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
		UE_LOG(LogTemp, Display, TEXT("Overlap Detected"));
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
		//ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
		UE_LOG(LogTemp, Display, TEXT("Unoverlap Detected"));
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
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		WeaponMesh->SetVisibility(true);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AreaBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//this is done to enable the physics on the strap, make sure done in onrep as well!
		if (WeaponType == EWeaponType::EWT_SubmachineGun) 
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(false);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			WeaponMesh->WakeAllRigidBodies();
		}
		EnableCustomDepth(false);
		break;

	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			AreaBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		AreaBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
		WeaponMesh->MarkRenderStateDirty();
		EnableCustomDepth(true);
		break;

	case EWeaponState::EWS_Stored:
		WeaponMesh->SetVisibility(false);
		break;
	}
	
}

//only one replicated var-> weap state
void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		WeaponMesh->SetVisibility(true);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(false);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			WeaponMesh->WakeAllRigidBodies();
		}
		EnableCustomDepth(false);
		break;
	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
		WeaponMesh->MarkRenderStateDirty();
		EnableCustomDepth(true);
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