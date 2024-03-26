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
#include "EngineUtils.h"
#include "Engine/PostProcessVolume.h"

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
	
	if (SpawnedPhysicsBox)
	{
		SpawnedPhysicsBox->SetCharacterMesh(Character->GetMesh());
	}

	Character->GetMesh()->SetSimulatePhysics(true);
	Character->GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	Character->GetMesh()->bReplicatePhysicsToAutonomousProxy = true;

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


void UDeathComponent::RadialBlur(float Intensity, float Duration)
{
	if (bIsRadialBlurActive) return;
	if (!TargetPostProcessVolume)
	{
		for (TActorIterator<APostProcessVolume> It(GetWorld()); It; ++It)
		{
				TargetPostProcessVolume = *It;
				break;
		}
	}


	if (TargetPostProcessVolume != nullptr) {
		UE_LOG(LogTemp, Log, TEXT("1"));

		auto& blendables = TargetPostProcessVolume->Settings.WeightedBlendables;
		// Ensure there is at least one element to avoid out-of-range errors.
		if (blendables.Array.Num() > 2) {
			UE_LOG(LogTemp, Log, TEXT("2"));
			auto* originalMat = Cast<UMaterialInstance>(blendables.Array[2].Object);
			UMaterialInstanceDynamic* dynamicMat = nullptr;

			// Check if it's already a UMaterialInstanceDynamic, else create a new one.
			if (originalMat != nullptr) {
				UE_LOG(LogTemp, Log, TEXT("3"));
				dynamicMat = Cast<UMaterialInstanceDynamic>(blendables.Array[2].Object);
				if (dynamicMat == nullptr) {
					UE_LOG(LogTemp, Log, TEXT("4"));
					dynamicMat = UMaterialInstanceDynamic::Create(originalMat, this);
					// Replace the original material with the new dynamic one.
					blendables.Array[2].Object = dynamicMat;
				}
				// Now, safely set the parameters on the dynamic material instance.
				if (dynamicMat != nullptr) {
					UE_LOG(LogTemp, Log, TEXT("5"));
					dynamicMat->SetScalarParameterValue(FName("Blur"), 2.f);
					dynamicMat->SetScalarParameterValue(FName("Blur Radius"), 1.f);
					dynamicMat->SetScalarParameterValue(FName("Density"), 6.f);
				}
			}
		}
	}
	/*
	
	if (!RadialBlurMaterialInstance)
	{
		if (RadialBlurMaterial)
		{
			RadialBlurMaterialInstance = UMaterialInstanceDynamic::Create(RadialBlurMaterial, this);
			if (TargetPostProcessVolume && RadialBlurMaterialInstance)
			{
				TargetPostProcessVolume->Settings.AddBlendable(RadialBlurMaterialInstance, 1.0f);
			}
		}
	}

	if (RadialBlurMaterialInstance)
	{
		// Ensure any previous effect adjustments are stopped before starting a new one
		GetWorld()->GetTimerManager().ClearTimer(PostProcessEffectTimerHandle);

		float StartTime = GetWorld()->GetTimeSeconds();
		float EndTime = StartTime + Duration;

		GetWorld()->GetTimerManager().SetTimer(PostProcessEffectTimerHandle, [this, StartTime, EndTime, Intensity, Duration]() mutable {
			if (!RadialBlurMaterialInstance) {
				GetWorld()->GetTimerManager().ClearTimer(PostProcessEffectTimerHandle);
				return;
			}
			float CurrentTime = GetWorld()->GetTimeSeconds();
			float Alpha = FMath::Clamp((CurrentTime - StartTime) / Duration, 0.0f, 1.0f);
			// Smooth step for a more natural ease in/out
			float SmoothAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

			// Calculated based on Intensity and SmoothAlpha
			float BlurValue = FMath::Lerp(0.0f, 2.0f, Intensity * SmoothAlpha);
			float BlurRadiusValue = FMath::Lerp(0.0f, 1.0f, Intensity * SmoothAlpha);
			float DensityValue = FMath::Lerp(0.0f, 6.0f, Intensity * SmoothAlpha);

			RadialBlurMaterialInstance->SetScalarParameterValue(FName("Blur"), BlurValue);
			RadialBlurMaterialInstance->SetScalarParameterValue(FName("Blur Radius"), BlurRadiusValue);
			RadialBlurMaterialInstance->SetScalarParameterValue(FName("Density"), DensityValue);
			if (CurrentTime >= EndTime)
			{
				GetWorld()->GetTimerManager().ClearTimer(PostProcessEffectTimerHandle);
				bIsRadialBlurActive = false;
			}
		}, 0.01f, true); // Update every 0.01 seconds (100 Hz) for smoother transitions
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("RadialBlurMaterialInstance is not valid."));
	}
	*/
}
