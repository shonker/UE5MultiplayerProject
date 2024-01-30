// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/BlasterComponents/InventoryComponent.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blaster/BlasterComponents/RagdollCube.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Blaster/Limb/Limb.h"
#include "Components/BoxComponent.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterAnimInstance.h"

// Sets default values for this component's properties
UDeathComponent::UDeathComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDeathComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UDeathComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UDeathComponent::MulticastElim_Implementation()
{
	if (!Character) return;

	Controller = Controller == nullptr ?
		Cast<ABlasterPlayerController>(Character->Controller) :
		Controller;
	Combat = Combat == nullptr ?
		Cast<UCombatComponent>(Character->GetCombat()) :
		Combat;

	if (Controller)
	{
		Controller->SetHUDWeaponAmmo(0);
	}
	Character->bElimmed = true;
	Character->bDisableGameplay = true;

	if (Character->HasAuthority() && PhysicsBoxBlueprint)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Owner = Character;

		SpawnedPhysicsBox = Character->GetWorld()->SpawnActor<ARagdollCube>(PhysicsBoxBlueprint, Character->GetActorLocation(), Character->GetActorRotation(), SpawnParams);
	}

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			if (SpawnedPhysicsBox)
			{
				Character->GetMesh()->AttachToComponent(SpawnedPhysicsBox->PhysicsBox, FAttachmentTransformRules::KeepWorldTransform);
				Character->GetMesh()->SetSimulatePhysics(true);
				Character->GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
				Character->GetMesh()->bReplicatePhysicsToAutonomousProxy = false;
				SpawnedPhysicsBox->SetCharacterMesh(Character->GetMesh());
			}
		}, 0.01f, false);
	Character->GetCharacterMovement()->DisableMovement();
	Character->GetCharacterMovement()->StopMovementImmediately();

	Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FTimerHandle CameraTransitionTimer;
	Character->GetWorldTimerManager().SetTimer(CameraTransitionTimer, this, &UDeathComponent::TransitionToSpectateCamera, CameraTransitionDelay, false);

	if (Controller)
	{
		Character->DisableInput(Controller);
	}

	if (Combat)
	{
		Combat->FireButtonPressed(false);
		Combat->RemoveHUDElements();
	}

}

void UDeathComponent::TransitionToSpectateCamera()
{
	if (!Character) return;
	TArray<AActor*> FoundPlayers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABlasterCharacter::StaticClass(), FoundPlayers);

	ABlasterCharacter* NextPlayerToSpectate = nullptr;
	for (AActor* Actor : FoundPlayers)
	{
		ABlasterCharacter* BlasterChar = Cast<ABlasterCharacter>(Actor);
		if (BlasterChar && BlasterChar != Character && !BlasterChar->bElimmed)
		{
			NextPlayerToSpectate = BlasterChar;
			break;
		}
	}

	if (NextPlayerToSpectate)
	{
		Character->CameraBoom->AttachToComponent(NextPlayerToSpectate->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Character->CameraBoom->TargetArmLength = 100.0f;
		Character->CameraBoom->SetWorldRotation(FRotator(-20.0f, 0.0f, 0.0f));
		Character->EnableInput(Controller);
	}
}

void UDeathComponent::TransitionToLimb()
{
	if (!Character) return;
	FVector SocketLocation = Character->GetMesh()->GetSocketLocation(FName("headSocket"));

	if (ALimb* LimbPawn = Character->GetWorld()->SpawnActor<ALimb>(Character->LimbClass, SocketLocation, Character->GetActorRotation()))
	{
		USpringArmComponent* PlayerCameraArm = Character->CameraBoom;
		USpringArmComponent* LimbCameraArm = LimbPawn->FindComponentByClass<USpringArmComponent>();

		float CameraTransitionSpeed = 5.0f;
		PlayerCameraArm->TargetArmLength = FMath::FInterpTo(PlayerCameraArm->TargetArmLength, LimbCameraArm->TargetArmLength, GetWorld()->GetDeltaSeconds(), CameraTransitionSpeed);
		PlayerCameraArm->SetWorldRotation(FMath::RInterpTo(PlayerCameraArm->GetComponentRotation(), LimbCameraArm->GetComponentRotation(), GetWorld()->GetDeltaSeconds(), CameraTransitionSpeed));

		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->Possess(LimbPawn);
		}
	}
}
