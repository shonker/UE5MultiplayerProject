

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
	if (bOrbOfLight) OrbOfLight();
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
	if (!BlasterOwnerCharacter->IsLocallyControlled()) return;

	switch (State)
	{
	case EWeaponState::EWS_PickedUp:
		RandomizeFOV();
		GetWorld()->GetTimerManager().SetTimer(FOVTimerHandle, [this]()
			{
				RandomizeFOV(); 
			}, FMath::RandRange(5.0f, 15.0f), true); 
		break;

	case EWeaponState::EWS_Dropped:
		GetWorld()->GetTimerManager().ClearTimer(FOVTimerHandle);
		
		float TargetFOV = 90.f;
		const float TransitionDuration = 2.0f; // Duration of the FOV transition in seconds
		const float UpdateInterval = 0.01f; //

		float StartTime = GetWorld()->GetTimeSeconds();
		GetWorld()->GetTimerManager().SetTimer(FOVInterpHandle, [this, StartTime, TransitionDuration, TargetFOV, UpdateInterval]() {
			float ElapsedTime = GetWorld()->GetTimeSeconds() - StartTime;
			float Alpha = FMath::Clamp(ElapsedTime / TransitionDuration, 0.0f, 1.0f);
			float NewFOV = FMath::Lerp(BlasterOwnerCharacter->GetFPSCamera()->FieldOfView, TargetFOV, Alpha);
			BlasterOwnerCharacter->GetFPSCamera()->SetFieldOfView(NewFOV);

			// Stop the timer if the transition is complete
			if (Alpha >= 1.0f)
			{
				GetWorld()->GetTimerManager().ClearTimer(FOVInterpHandle);
			}
			}, UpdateInterval, true);
		break;
	}
}
void AItem::RandomizeFOV()
{
	if (!BlasterOwnerCharacter) return;

	if (LongBreathCue)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LongBreathCue, BlasterOwnerCharacter->GetActorLocation());
	}

	float InitialFOV = BlasterOwnerCharacter->GetFPSCamera()->FieldOfView;
	float TargetFOV = 90.f; // FMath::RandRange(10.0f, 40.0f);
	constexpr float TransitionDuration = 2.0f; // Duration of the FOV transition in seconds
	constexpr float UpdateInterval = 0.01f; // Update interval in seconds
	float MaxOvershootFOV = FMath::Max(InitialFOV, TargetFOV) - FMath::RandRange(30.f, 70.0f);// 20.0f; // This ensures the overshoot is above both initial and target

	// Start a new FOV transition
	float StartTime = GetWorld()->GetTimeSeconds();
	GetWorld()->GetTimerManager().SetTimer(FOVInterpHandle, [this, StartTime, TransitionDuration, InitialFOV, TargetFOV, MaxOvershootFOV, UpdateInterval]() {
		float ElapsedTime = GetWorld()->GetTimeSeconds() - StartTime;
		float Alpha = FMath::Clamp(ElapsedTime / TransitionDuration, 0.0f, 1.0f);

		// Use the first half of a sine wave to go from InitialFOV to MaxOvershootFOV
		// Then use the second half to go from MaxOvershootFOV to TargetFOV
		float NewFOV;
		if (Alpha < 0.5f) {
			// Scale alpha to [0, 1] for the first half of the transition
			float ScaledAlpha = Alpha / 0.5f;
			NewFOV = FMath::Lerp(InitialFOV, MaxOvershootFOV, FMath::Sin(ScaledAlpha * PI * 0.5f));
		}
		else {
			// Scale alpha to [0, 1] for the second half of the transition
			float ScaledAlpha = (Alpha - 0.5f) / 0.5f;
			NewFOV = FMath::Lerp(MaxOvershootFOV, TargetFOV, 1 - FMath::Cos(ScaledAlpha * PI * 0.5f));
		}

		BlasterOwnerCharacter->GetFPSCamera()->SetFieldOfView(NewFOV);

		// Stop the timer if the transition is complete
		if (Alpha >= 1.0f)
		{
			GetWorld()->GetTimerManager().ClearTimer(FOVInterpHandle);
		}
		}, UpdateInterval, true);
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

	if (BlasterOwnerCharacter && HealthSapCue && FMath::RandBool())
	{
		UGameplayStatics::PlaySoundAtLocation(this, HealthSapCue, BlasterOwnerCharacter->GetActorLocation());
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
			GetWorld()->GetTimerManager().SetTimer(KnockingSoundTimerHandle, this, &AItem::PlayKnockingSound, FMath::RandRange(10,20), true, 3.0f); // Every 5 seconds
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
		PlaySoundCueAtRandomLocation(KnockingCue);
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
			BlasterOwnerCharacter->GetWorld()->GetTimerManager().SetTimer(MurmuringSoundTimerHandle, this, &AItem::PlayMurmuringSound, FMath::RandRange(12,30), true, 7.0f); // Every 7 seconds
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
		PlaySoundCueAtRandomLocation(MurmurCue);
	}
}

void AItem::Laughing(EWeaponState State)
{
	switch (State)
	{
	case EWeaponState::EWS_PickedUp:
		if (BlasterOwnerCharacter)
		{
			int32 RandomTime = FMath::RandRange(1, 20);
			int32 LaughType = FMath::RandRange(0, 2);

			// SetTimer using a lambda function
			BlasterOwnerCharacter->GetWorld()->GetTimerManager().SetTimer(
				LaughingSoundTimerHandle,
				[this, LaughType]() { this->PlayLaughingSound(LaughType); },
				RandomTime,
				true,
				1.0f
			);
		}
		break;

	case EWeaponState::EWS_Dropped:
		BlasterOwnerCharacter->GetWorld()->GetTimerManager().ClearTimer(LaughingSoundTimerHandle);
		break;

	case EWeaponState::EWS_Stored:
		break;
	}
}

void AItem::PlayLaughingSound(int32 OptionalLaughType)
{
	if (BlasterOwnerCharacter)
	{
		USoundCue* ChosenCue = nullptr;
		int SoundChosen = (OptionalLaughType >= 0 && OptionalLaughType <= 2) ? OptionalLaughType : FMath::RandRange(0, 2);

		switch (SoundChosen)
		{
		case 0:
			ChosenCue = ManLaughingCue;
			break;
		case 1:
			ChosenCue = WomanLaughingCue;
			break;
		case 2:
			ChosenCue = ChildLaughingCue;
			break;
		}

		if (ChosenCue != nullptr)
		{
			PlaySoundCueAtRandomLocation(ChosenCue);
		}
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


void AItem::PlaySoundCueAtRandomLocation(USoundCue* SoundCue)
{
	if (BlasterOwnerCharacter && SoundCue)
	{
		FVector RandomDirection = FMath::VRand();
		float Radius = 700.0f;
		FVector SoundLocation = BlasterOwnerCharacter->GetActorLocation() + (RandomDirection * FMath::RandRange(0.0f, Radius));
		UGameplayStatics::PlaySoundAtLocation(this, SoundCue, SoundLocation);
	}
}

void AItem::OrbOfLight()
{
	UWorld* World = GetWorld();
	if (World && OrbOfLightBlueprint)
	{
		GetWorld()->SpawnActor<AActor>(OrbOfLightBlueprint, GetActorTransform());
		bOrbOfLight = false;
	}
}