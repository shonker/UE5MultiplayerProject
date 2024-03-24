

#include "Item.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"

void AItem::BeginPlay()
{
	Super::BeginPlay();
}

void AItem::SpendRound()
{
	//do nothing, no ammo in this context
}

void AItem::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
}

void AItem::OnSetWeaponState(EWeaponState State)
{
	Super::OnSetWeaponState(State);
	UpdateCurse(State);
	//each curse needs to be structured as such:
	/*
		switch (State)
		{
		case EWeaponState::EWS_PickedUp:
		//curse activates
			break;

		case EWeaponState::EWS_Dropped:
		//curse deactivates immediately, over time, or not until taken to home base
			break;

		case EWeaponState::EWS_Stored:
			break;
		}
	*/
}

void AItem::UpdateCurse(EWeaponState State)
{
	BlasterOwnerCharacter =
		BlasterOwnerCharacter == nullptr ?
		Cast<ABlasterCharacter>(GetOwner()) :
		BlasterOwnerCharacter;

	if (!BlasterOwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("Blaster Owner Character inaccessible??"));
		return;
	}
	else
	{
		BlasterOwnerController =
			BlasterOwnerController == nullptr ?
			Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) :
			BlasterOwnerController;
	}

	//HARD
	if (bBlindness) Blindness(State);
	if (bHealthSap) HealthSap(State);
	if (bReversedMovementControls) ReversedMovementControls(State);
	if (bReversedLookControls) ReversedLookControls(State);
	if (bActivateKisser) ActivateKisser(State);

	// MEDIUM
	if (bFetus) Fetus(State);
	if (bHeavyJumping) HeavyJumping(State);
	if (bCountdownPain) CountdownPain(State);
	if (bFallDamage) FallDamage(State);
	if (bShiftMovementControls) ShiftMovementControls(State);

	// EASY
	if (bKnocking) Knocking(State);
	if (bMurmuring) Murmuring(State);
	if (bLaughing) Laughing(State);
	if (bChangingFOV) ChangingFOV(State);
	if (bScreenShake) ScreenShake(State);
	//if (bCastsRedLight) CastsRedLight(State);
	//if (bAutomaticDecapitation) AutomaticDecapitation(State);
}

void AItem::HeavyJumping(EWeaponState State)
{
	if (!BlasterOwnerCharacter) return;
	switch (State)
	{
	case EWeaponState::EWS_PickedUp:
			// Reduce jump Z velocity to make jumps less effective
			BlasterOwnerCharacter->GetCharacterMovement()->JumpZVelocity *= 0.5;
		break;

	case EWeaponState::EWS_Dropped:
			// Reset jump Z velocity to normal
			BlasterOwnerCharacter->GetCharacterMovement()->JumpZVelocity /= 0.5;
		break;

	case EWeaponState::EWS_Stored:
		break;
	}
}

void AItem::Blindness(EWeaponState State)
{
	if (!BlasterOwnerCharacter) return;
	switch (State)
	{
	case EWeaponState::EWS_PickedUp:
			BlasterOwnerCharacter->GetFollowCamera()->SetFieldOfView(10.f);
		break;

	case EWeaponState::EWS_Dropped:
			BlasterOwnerCharacter->GetFollowCamera()->SetFieldOfView(90.f);
		break;

	case EWeaponState::EWS_Stored:
		break;
	}
}

void AItem::HealthSap(EWeaponState State)
{
	if (!BlasterOwnerCharacter) return;
	switch (State)
	{
	case EWeaponState::EWS_PickedUp:
		GetWorld()->GetTimerManager().SetTimer(HealthSapTimerHandle, this, &AItem::DecreaseHealth, 2.0f, true, 2.0f);
		break;

	case EWeaponState::EWS_Dropped:
		GetWorld()->GetTimerManager().ClearTimer(HealthSapTimerHandle);
		break;

	case EWeaponState::EWS_Stored:
		// No action needed for stored state in this case
		break;
	}
}

void AItem::DecreaseHealth()
{
	if (!BlasterOwnerCharacter) return;

	BlasterOwnerController =
	BlasterOwnerController == nullptr ?
	Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) :
	BlasterOwnerController;

	if (BlasterOwnerController)
	{
		UGameplayStatics::ApplyDamage(
			BlasterOwnerCharacter,
			1.f,
			BlasterOwnerController,
			this,
			UDamageType::StaticClass()
		);
	}
}

void AItem::ReversedMovementControls(EWeaponState State)
{
	if (!BlasterOwnerCharacter) return;
	switch (State)
	{
	case EWeaponState::EWS_PickedUp:
			BlasterOwnerCharacter->SetReversedMovementControls(true);
		break;

	case EWeaponState::EWS_Dropped:
			BlasterOwnerCharacter->SetReversedMovementControls(false);
		break;

	case EWeaponState::EWS_Stored:
		break;
	}
}

void AItem::ReversedLookControls(EWeaponState State)
{
	if (!BlasterOwnerCharacter) return;
	switch (State)
	{
	case EWeaponState::EWS_PickedUp:
			BlasterOwnerCharacter->SetReversedLookControls(true);
		break;

	case EWeaponState::EWS_Dropped:
			BlasterOwnerCharacter->SetReversedLookControls(false);
		break;

	case EWeaponState::EWS_Stored:
		break;
	}
}

void AItem::ActivateKisser(EWeaponState State)
{
	if (!BlasterOwnerCharacter) return;
	switch (State)
	{
	case EWeaponState::EWS_PickedUp:
		break;

	case EWeaponState::EWS_Dropped:
		break;

	case EWeaponState::EWS_Stored:
		break;
	}
}

void AItem::Fetus(EWeaponState State)
{
	if (!BlasterOwnerCharacter) return;
	switch (State)
	{
	case EWeaponState::EWS_PickedUp:
		break;

	case EWeaponState::EWS_Dropped:
		break;

	case EWeaponState::EWS_Stored:
		break;
	}
}

void AItem::CountdownPain(EWeaponState State)
{
	if (!BlasterOwnerCharacter) return;
	switch (State)
	{
	case EWeaponState::EWS_PickedUp:
		GetWorld()->GetTimerManager().SetTimer(CountdownPainTimerHandle, this, &AItem::TriggerCountdownPain, 10.0f, true, 10.0f); // Trigger every 10 seconds
		break;

	case EWeaponState::EWS_Dropped:
		GetWorld()->GetTimerManager().ClearTimer(CountdownPainTimerHandle);
		break;

	case EWeaponState::EWS_Stored:
		break;
	}
}

void AItem::TriggerCountdownPain()
{
	if (BlasterOwnerCharacter)
	{
		const float DamageAmount = 10.0f;
		BlasterOwnerCharacter->TakeDamage(DamageAmount, FDamageEvent(), nullptr, nullptr);
	}
}

void AItem::FallDamage(EWeaponState State)
{
	if (!BlasterOwnerCharacter) return;
	switch (State)
	{
	case EWeaponState::EWS_PickedUp:
		BlasterOwnerCharacter->SetTakesFallDamage(true); 
		break;

	case EWeaponState::EWS_Dropped:
		BlasterOwnerCharacter->SetTakesFallDamage(false);
		break;

	case EWeaponState::EWS_Stored:
		break;
	}
}

void AItem::ShiftMovementControls(EWeaponState State)
{
	if (!BlasterOwnerCharacter) return;
	switch (State)
	{
	case EWeaponState::EWS_PickedUp:
			BlasterOwnerCharacter->SetShiftMovementControls(true);
		break;

	case EWeaponState::EWS_Dropped:
			BlasterOwnerCharacter->SetShiftMovementControls(false);
		break;

	case EWeaponState::EWS_Stored:
		break;
	}
}

void AItem::Knocking(EWeaponState State)
{
	switch (State)
	{
	case EWeaponState::EWS_PickedUp:
		if (BlasterOwnerCharacter)
		{
			// Start a repeating timer to play knocking sound
			GetWorld()->GetTimerManager().SetTimer(KnockingSoundTimerHandle, this, &AItem::PlayKnockingSound, 5.0f, true, 5.0f); // Every 5 seconds
		}
		break;

	case EWeaponState::EWS_Dropped:
		// Stop the knocking sound effect
		GetWorld()->GetTimerManager().ClearTimer(KnockingSoundTimerHandle);
		break;

	case EWeaponState::EWS_Stored:
		break;
	}
}

void AItem::PlayKnockingSound()
{
	if (BlasterOwnerCharacter && KnockingCue)
	{
		UGameplayStatics::PlaySoundAtLocation(this, KnockingCue, BlasterOwnerCharacter->GetActorLocation());
	}
}

void AItem::Murmuring(EWeaponState State)
{
	switch (State)
	{
	case EWeaponState::EWS_PickedUp:
		if (BlasterOwnerCharacter)
		{
			// Start a repeating timer to play murmuring sound
			BlasterOwnerCharacter->GetWorld()->GetTimerManager().SetTimer(MurmuringSoundTimerHandle, this, &AItem::PlayMurmuringSound, 7.0f, true, 7.0f); // Every 7 seconds
		}
		break;

	case EWeaponState::EWS_Dropped:
		// Stop the murmuring sound effect
		BlasterOwnerCharacter->GetWorld()->GetTimerManager().ClearTimer(MurmuringSoundTimerHandle);
		break;

	case EWeaponState::EWS_Stored:
		break;
	}
}

void AItem::PlayMurmuringSound()
{
	if (BlasterOwnerCharacter && MurmurCue)
	{
		UGameplayStatics::PlaySoundAtLocation(this, MurmurCue, BlasterOwnerCharacter->GetActorLocation());
	}
}

void AItem::Laughing(EWeaponState State)
{
	switch (State)
	{
	case EWeaponState::EWS_PickedUp:
		if (BlasterOwnerCharacter)
		{
			BlasterOwnerCharacter->GetWorld()->GetTimerManager().SetTimer(LaughingSoundTimerHandle, this, &AItem::PlayLaughingSound, FMath::RandRange(1,20), true, 1.0f);
		}
		break;

	case EWeaponState::EWS_Dropped:
		BlasterOwnerCharacter->GetWorld()->GetTimerManager().ClearTimer(LaughingSoundTimerHandle);
		break;

	case EWeaponState::EWS_Stored:
		break;
	}
}

void AItem::PlayLaughingSound()
{
	if (BlasterOwnerCharacter && LaughingCue)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LaughingCue, BlasterOwnerCharacter->GetActorLocation());
	}
}

void AItem::ChangingFOV(EWeaponState State)
{
	switch (State)
	{
	case EWeaponState::EWS_PickedUp:
		if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetFollowCamera())
		{
			// Change FOV to a different value
			BlasterOwnerCharacter->GetFollowCamera()->SetFieldOfView(110.f); // Example FOV change
		}
		break;

	case EWeaponState::EWS_Dropped:
		if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetFollowCamera())
		{
			// Reset FOV to normal
			BlasterOwnerCharacter->GetFollowCamera()->SetFieldOfView(90.f); // Assuming 90 is the default FOV
		}
		break;

	case EWeaponState::EWS_Stored:
		// No action needed for stored state
		break;
	}
}

void AItem::ScreenShake(EWeaponState State)
{
	switch (State)
	{
	case EWeaponState::EWS_PickedUp:
		if (BlasterOwnerCharacter)
		{
			// Start a repeating timer to trigger screen shake
			BlasterOwnerCharacter->GetWorld()->GetTimerManager().SetTimer(ScreenShakeTimerHandle, this, &AItem::TriggerScreenShake, FMath::RandRange(1,30), true, 0.5f); 
		}
		break;

	case EWeaponState::EWS_Dropped:
		// Stop the screen shake effect
		BlasterOwnerCharacter->GetWorld()->GetTimerManager().ClearTimer(ScreenShakeTimerHandle);
		break;

	case EWeaponState::EWS_Stored:
		// No action needed for stored state
		break;
	}
}

void AItem::TriggerScreenShake()
{
	if (BlasterOwnerCharacter)
	{
		//// Assuming a generic screen shake class exists; replace with your actual screen shake class
		//UCameraShakeBase* MyScreenShake = NewObject<UCameraShakeBase>(BlasterOwnerCharacter, UMyScreenShake::StaticClass());
		//BlasterOwnerCharacter->GetPlayerController()->ClientStartCameraShake(MyScreenShake);
	}
}
