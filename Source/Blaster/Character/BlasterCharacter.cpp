
// Fill out your copyright notice in the Description page of Project Settings.
#include "BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Buttons/MyButton.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/BlasterComponents/InventoryComponent.h" 
#include "Blaster/BlasterComponents/RagdollCube.h" 
#include "Components/BoxComponent.h"
#include "Blaster/BlasterComponents/DeathComponent.h" 
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "BlasterAnimInstance.h"
#include "Blaster/Blaster.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "TimerManager.h"
#include "Blaster/Limb/Limb.h"
#include "Blaster/BlasterPlayerState/BlasterPlayerState.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/SphereComponent.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "DrawDebugHelpers.h"


ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 350.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	

	FPSBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("FPSBoom"));
	FPSBoom->SetupAttachment(GetMesh());
	FPSBoom->TargetArmLength = 1.f;
	FPSBoom->bUsePawnControlRotation = true;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(FPSBoom, USpringArmComponent::SocketName);
	FirstPersonCamera->bUsePawnControlRotation = false;

	//this also has to be set in BP as BP will override them
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);  

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Death = CreateDefaultSubobject<UDeathComponent>(TEXT("DeathComponent"));
	Death->Combat = Combat;
	Death->SetIsReplicated(true);

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	InventoryComponent->Combat = Combat;
	Combat->InventoryComponent = InventoryComponent;

	//this is also checked on bp character movement comp in myblasterchar
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	InteractSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractSphere"));
	InteractSphere->SetupAttachment(RootComponent);
	InteractSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	InteractSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	VisualTargetSphere = CreateDefaultSubobject<USphereComponent>(TEXT("VisualTargetSphere"));
	VisualTargetSphere->SetupAttachment(RootComponent);
	VisualTargetSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	VisualTargetSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//this prevents the camera from moving when another player is between camera and player mesh
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingButton, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingBody, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingFriend, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, InteractTargetLocation, COND_SkipOwner);
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = BlasterGameMode 
		&& BlasterGameMode->GetMatchState() != MatchState::InProgress;
	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateHUDHealth();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
		
		GetMesh()->OnComponentBeginOverlap.AddDynamic(this, &ABlasterCharacter::OnInteractSphereOverlap);
		GetMesh()->OnComponentEndOverlap.AddDynamic(this, &ABlasterCharacter::OnInteractSphereEndOverlap);

		InteractSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		InteractSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		VisualTargetSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		VisualTargetSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	}

	if (FirstPersonCamera)
	{
		FollowCamera->SetActive(false);
		FirstPersonCamera->SetActive(true);
		FirstPersonCamera->bUsePawnControlRotation = false;

		//this also has to be set in BP as BP will override them
		bUseControllerRotationYaw = true;
		if (IsLocallyControlled())
		{
			bUseControllerRotationPitch = false;
		}
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
}

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsLocallyControlled() && !bElimmed)
	{
		ManageVisualInteractionTargetLocations();
	}
	RotateInPlace(DeltaTime);
	AimOffset(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();
}

void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Equip", IE_Released, this, &ABlasterCharacter::EquipButtonReleased);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ABlasterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);
	
	PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &ABlasterCharacter::ThrowButtonPressed);
	PlayerInputComponent->BindAction("Throw", IE_Released, this, &ABlasterCharacter::ThrowButtonReleased);


	PlayerInputComponent->BindAction("ItemShuffleLeft", IE_Pressed, this, &ABlasterCharacter::ItemShuffleLeft);
	PlayerInputComponent->BindAction("ItemShuffleRight", IE_Pressed, this, &ABlasterCharacter::ItemShuffleRight);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
	if (Death)
	{
		Death->Character = this;
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

//so the way this works for the future
//is there is a reload montage in the blueprints/character/animations folder
//drag in an animation after the rifle animation, right click->add section (ig: "shotgun")
//come here and add it to the switch case
//make sure its also added the WeaponTypes.h enum as well
void ABlasterCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;	
		
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("Rifle");
			break;
		
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Rifle");
			break;
		
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Rifle");
			break;
		
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;

		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("Rifle");
			break;

		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("Rifle");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::Elim()
{
	if (InventoryComponent)
	{
		InventoryComponent->RemoveAllItems();
	}
	Death->MulticastElim();
}

void ABlasterCharacter::ThrowButtonPressed()
{
	if (Combat)
	{
		Combat->StartThrowCharging();
	}
}

void ABlasterCharacter::ThrowButtonReleased()
{
	if (Combat)
	{
		Combat->Throw();
	}
}

void ABlasterCharacter::StartDissolve() //and death particles lol
{
	// Create the particle system
	if (DeathParticles)
	{
		USkeletalMeshComponent* MeshComponent = GetMesh(); 
		if (MeshComponent)
		{
			// Use the "spine_0003Socket" socket name to attach the particle system
			FName SocketName("spine_003Socket");

			UParticleSystemComponent* ParticleComponent = UGameplayStatics::SpawnEmitterAttached(
				DeathParticles, // The particle system you want to spawn
				MeshComponent,        // The mesh to which you want to attach the particle system
				SocketName,     // Replace with the socket name or empty string to attach to the mesh directly
				FVector(0, 0, 0), // Relative location (offset) of the particle system
				FRotator(0, 0, 0), // Relative rotation of the particle system
				EAttachLocation::SnapToTarget, // Attachment rule
				true // Auto destroy
			);
		}
		// You can further configure the ParticleComponent here, e.g., set parameters.
	}

	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}

	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::ElimTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterGameMode)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::PlayThrowMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowMontage)
	{
		AnimInstance->Montage_Play(ThrowMontage);
	}
}
void ABlasterCharacter::PlayKissMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && KissMontage)
	{
		AnimInstance->Montage_Play(KissMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	if (!(GetCombatState() == ECombatState::ECS_Unoccupied)) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

//only called on server
void ABlasterCharacter::ReceiveDamage(
	AActor* DamagedActor, 
	float Damage, 
	const UDamageType* DamageType, 
	AController* InstigatorController, 
	AActor* DamageCauser)
{
	//GIVE BLOOD FOR HITS
	ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
	if (!AttackerController) return;
	ABlasterPlayerState* AttackerPlayerState =
		AttackerController ?
		Cast<ABlasterPlayerState>(AttackerController->PlayerState)
		: nullptr;
	if (AttackerPlayerState)
	{
			AttackerPlayerState->AddToScore(1.666f);
	}

	//UPDATE HEALTH
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();

	//KILL
	if (Health <= 0.f)
	{
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if (BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}

}

void ABlasterCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		//Return an f vector only containing the rotation on the x axis, zero'd out on the pitch and roll
		const FVector Direction( FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		//this only tells the system that movement input is applied
		//speed and direction belong in the char movement component
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0.f)
	{
		//same as move forward, BUT isolate Y axis
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
	
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ABlasterCharacter::EquipButtonPressed()
{
	if (bDisableGameplay) return;

	if (HasAuthority())
	{
		HandleEquipButtonPressed();
	}
	else
	{
		ServerEquipButtonPressed();
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	HandleEquipButtonPressed();
}

void ABlasterCharacter::HandleEquipButtonPressed()
{
	if (InventoryComponent)
	{
		if (OverlappingWeapon)
		{
			InventoryComponent->AddItem(OverlappingWeapon);
			return;
		}
	}
	if (OverlappingButton)
	{
		OverlappingButton->OnInitPress();
		return;
	}
	if (OverlappingFriend)
	{
		Kiss(true);
		return;
	}
	if (OverlappingBody && OverlappingBody->PhysicsBox)
	{
		if (OverlappingBody->FollowChar)
		{ 
			OverlappingBody->bFollowPlayer = false;
			OverlappingBody->FollowChar = false;
		}
		else
		{
			OverlappingBody->bFollowPlayer = true;
			OverlappingBody->FollowChar = this;
			OverlappingBody->GrabBodyAtSocket();
			return;
		}
	}
}

void ABlasterCharacter::HandleEquipButtonReleased()
{
	if (OverlappingButton)
	{
		OverlappingButton->OnRelease();
	}
	Kiss(false);
}

void ABlasterCharacter::Kiss(bool StartOrEnd)
{
	if (Combat == nullptr) return;
	if (StartOrEnd)//starting
	{
		Combat->StartKissCharging();
	}
	else//ending
	{
		Combat->Kiss();
	}
}

void ABlasterCharacter::EquipButtonReleased()
{
	if (bDisableGameplay) return;
	if (HasAuthority())
	{
		HandleEquipButtonReleased();
	}
	else
	{
		ServerEquipButtonReleased();
	}
	
}

void ABlasterCharacter::ServerEquipButtonReleased_Implementation()
{
	HandleEquipButtonReleased();
}

void ABlasterCharacter::ItemShuffleLeft()
{ //this is so fucking stupid
		ServerShuffleItem(true);
}

void ABlasterCharacter::ItemShuffleRight()
{
		ServerShuffleItem(false);
}

void ABlasterCharacter::ServerShuffleItem_Implementation(bool IsLeft)
{
	if (InventoryComponent)
	{
		InventoryComponent->ShuffleItem(IsLeft);
	}
}

//luckily, since we use the built in crouch functionality, crouching is already set up to be replicated
void ABlasterCharacter::CrouchButtonPressed()
{ 
	if (bDisableGameplay) return;

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}

}

void ABlasterCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		Combat->Reload();
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if (bDisableGameplay) return;

	//replication of bAiming handled from server->client in CombatComponent.cpp DOREPLIFETIME
	//replication of client-> server bAiming handled with RPC in 
	if (Combat)
	{
		//SP imp
		//Combat->bAiming = true;

		//mp imp
		Combat->SetAiming(true);

	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		//Combat->bAiming = false;
		Combat->SetAiming(false);
	}
}	

float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (bDisableGameplay) return;
	//if (Combat && Combat->EquippedWeapon == nullptr)
	//{
	//	return;
	//}

	bool bIsInAir = GetCharacterMovement()->IsFalling();
	float Speed = CalculateSpeed();
	if (Speed == 0.f && !bIsInAir) //standing still && not jumping
	{
	//	bRotateRootBone = true;
	//	FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
	//	FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
	//	AO_Yaw = DeltaAimRotation.Yaw;
	//	if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
	//	{
	//		InterpAO_Yaw = AO_Yaw;
	//	}
	//	bUseControllerRotationYaw = true;
	TurnInPlace(DeltaTime);
	}
	//if (Speed > 0.f || bIsInAir) //running || jumping
	//{
	//	bRotateRootBone = false;
	//	bUseControllerRotationYaw = true;
	//	StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
	//	AO_Yaw = 0.f;
	//	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	//}

	CalculateAO_Pitch();
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//map pitch from [270, 360) to [-90, 0)
		//bitwise compression for var replication takes negative numbers 
		//and loops them back to 360 descending
		//this is demonstrated in the CharMoveComp
		//but for our aim offset blendspaces we must correct that
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlasterCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;

	//float Speed = CalculateSpeed();
	//if (Speed > 0.f)
	//{
	//	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	//	return;
	//}

	//ProxyRotationLastFrame = ProxyRotation;
	//ProxyRotation = GetActorRotation();
	//ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	//if (FMath::Abs(ProxyYaw) > TurnThreshold)
	//{
	//	if (ProxyYaw > TurnThreshold)
	//	{
	//		TurningInPlace = ETurningInPlace::ETIP_Right;
	//	}
	//	else if (ProxyYaw < -TurnThreshold)
	//	{
	//		TurningInPlace = ETurningInPlace::ETIP_Left;
	//	}
	//	else
	//	{
	//		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	//	}
	//	return;
	//}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

}

void ABlasterCharacter::Jump()
{
	if (bDisableGameplay) return;

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::FireButtonPressed()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 45.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -45.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		//InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		//AO_Yaw = InterpAO_Yaw;
		///*if (FMath::Abs(AO_Yaw) < 15.f)
		//{*/
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);

		//}
	}
}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			//Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ABlasterCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		//this isn't a cast? something about "templates"
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDebt(0.f);
		}
	}
}

void ABlasterCharacter::ManageVisualInteractionTargetLocations()
{
	FVector_NetQuantize CurrentHitTarget = GetHitTarget();
	if (LastHitTarget != CurrentHitTarget)
	{
		ServerSetInteractTarget(CurrentHitTarget);
		SetInteractAndVisualTargetSphereLocation(CurrentHitTarget);
		LastHitTarget = CurrentHitTarget;
	}
}

void ABlasterCharacter::SetInteractAndVisualTargetSphereLocation(FVector_NetQuantize Target)
{
	FVector_NetQuantize CharacterLocation = GetActorLocation();
	float Distance = FVector::Dist(CharacterLocation, Target);
	if (IsLocallyControlled() || HasAuthority())
	{
		if (Distance < 156.f)
		{
			InteractSphere->SetWorldLocation(Target);
		}
		else
		{
			InteractSphere->SetWorldLocation(CharacterLocation);
		}
	}
	VisualTargetSphere->SetWorldLocation(Target);
}

void ABlasterCharacter::ServerSetInteractTarget_Implementation(FVector_NetQuantize InteractTarget)
{
	InteractTargetLocation = InteractTarget;
	SetInteractAndVisualTargetSphereLocation(InteractTarget);
}

void ABlasterCharacter::OnRep_InteractTargetLocation()
{
	SetInteractAndVisualTargetSphereLocation(InteractTargetLocation);
}


void ABlasterCharacter::ClearHUDInteractText()
{
	if (BlasterPlayerController)
	{
		FString EmptyText(TEXT(""));
		BlasterPlayerController->SetHUDInteractText(EmptyText);
	}
}

void ABlasterCharacter::SetOverlappingButton(AMyButton* Button)
{
	OverlappingButton = Button;
	if (IsLocallyControlled())
	{
		if (OverlappingButton)
		{
			if (BlasterPlayerController)
			{
				BlasterPlayerController->SetHUDInteractText(OverlappingButton->InteractionText);
			}
		}
		else
		{
			ClearHUDInteractText();
		}
	}
}

void ABlasterCharacter::OnRep_OverlappingButton(AMyButton* LastButton)
{
	if (OverlappingButton) //this is the new var, LatWeapon is the old one
	{
		if (BlasterPlayerController)
		{
			BlasterPlayerController->SetHUDInteractText(OverlappingButton->InteractionText);
		}
	}
	else
	{
		ClearHUDInteractText();
	}
}


void ABlasterCharacter::SetOverlappingBody(ARagdollCube* Cube)
{
	OverlappingBody = Cube;

	if (IsLocallyControlled())
	{
		if (OverlappingBody)
		{
			if (BlasterPlayerController)
			{
				FString Interaction(TEXT("DRAG BODY"));
				BlasterPlayerController->SetHUDInteractText(Interaction);
			}
		}
		else
		{
			ClearHUDInteractText();
		}
	}
}


void ABlasterCharacter::OnRep_OverlappingBody(ARagdollCube* LastCube)
{
	if (OverlappingBody) //this is the new var, LatWeapon is the old one
	{
		if (BlasterPlayerController)
		{
			FString Interaction(TEXT("DRAG BODY"));
			BlasterPlayerController->SetHUDInteractText(Interaction);
		}
	}
	else
	{
		ClearHUDInteractText();
	}
}


void ABlasterCharacter::SetOverlappingFriend(ABlasterCharacter* Friend)
{
	OverlappingFriend = Friend;

	if (IsLocallyControlled())
	{
		if (OverlappingFriend)
		{
			if (BlasterPlayerController)
			{
				FString Interaction(TEXT("KISS <3"));
				BlasterPlayerController->SetHUDInteractText(Interaction);
			}
		}
		else
		{
			ClearHUDInteractText();
		}
	}
}


void ABlasterCharacter::OnRep_OverlappingFriend()
{
	if (OverlappingFriend) //this is the new var, LatWeapon is the old one
	{
		if (BlasterPlayerController)
		{
			FString Interaction(TEXT("KISS <3"));
			BlasterPlayerController->SetHUDInteractText(Interaction);
		}
	}
	else
	{
		ClearHUDInteractText();
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon *Weapon)
{	
	OverlappingWeapon = Weapon;

	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			//OverlappingWeapon->ShowPickupWidget(true);
			if (BlasterPlayerController)
			{
				BlasterPlayerController->SetHUDInteractText(OverlappingWeapon->InteractionText);
			}
		}
		else
		{
			ClearHUDInteractText();
		}
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		if (BlasterPlayerController)
		{
			BlasterPlayerController->SetHUDInteractText(OverlappingWeapon->InteractionText);
		}
	}
	else
	{
		ClearHUDInteractText();
	}
}

void ABlasterCharacter::OnInteractSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == this) return;
	if (OtherComp && OtherComp->GetName() == FString("InteractSphere"))
	{
		ABlasterCharacter* BChar = Cast<ABlasterCharacter>(OtherActor);
		if (BChar)
		{
			BChar->SetOverlappingFriend(this);
		}
	}
}

void ABlasterCharacter::OnInteractSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == this) return;
	if (OtherComp && OtherComp->GetName() == FString("InteractSphere"))
	{
		ABlasterCharacter* BChar = Cast<ABlasterCharacter>(OtherActor);
		if (BChar)
		{
			BChar->SetOverlappingFriend(nullptr);
		}
	}
}
//currently called by anim instance
bool ABlasterCharacter::IsWeaponEquipped()
{
    return (Combat && Combat->EquippedWeapon);
}

//another getter accessed by anim instance
bool ABlasterCharacter::IsAiming()
{
    return(Combat && Combat->bAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
    return Combat->HitTarget;
}

ECombatState ABlasterCharacter::GetCombatState()
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}

