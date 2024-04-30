#include "Knight.h"
#include "KnightAnimInstance.h"
#include "Blaster/Buttons/MyButton.h"

// Sets default values
AKnight::AKnight()
{
	// Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create and set up the root component
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(RootComp);

	// Create and set up the knight mesh component
	KnightMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("KnightMesh"));
	KnightMesh->SetupAttachment(RootComponent);
	KnightMesh->SetAnimInstanceClass(UKnightAnimInstance::StaticClass()); // Ensure you have set the correct class

	// Create and set up the sword mesh component
	SwordMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SwordMesh"));
	SwordMesh->SetupAttachment(RootComponent);
	SwordMesh->SetAnimInstanceClass(USwordAnimInstance::StaticClass()); // Ensure you have set the correct class

	SwordButtonComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("Sword Button Component"));
	SwordButtonComponent->SetupAttachment(RootComp);
	SwordButtonComponent->SetChildActorClass(AMyButton::StaticClass());
	SwordButtonComponent->SetIsReplicated(true);
}

void AKnight::SwordButtonPressed()
{
	BPSwordButtonPressed();
	bButtonHeld = true;
}

void AKnight::SwordButtonDraggedOff()
{
	BPSwordButtonDraggedOff();
	bButtonHeld = false;
}

void AKnight::SwordButtonReleased()
{
	BPSwordButtonReleased();
	bButtonHeld = false;
}

// Called when the game starts or when spawned
void AKnight::BeginPlay()
{
	Super::BeginPlay();

	if (SwordButtonComponent && SwordButtonComponent->GetChildActor())
	{
		AMyButton* SwordButton = Cast<AMyButton>(SwordButtonComponent->GetChildActor());
		if (SwordButton)
		{
			SwordButton->InteractionText = TEXT("BEGIN DREAM");
			SwordButton->OnButtonPressed.AddDynamic(this, &AKnight::SwordButtonPressed);
			SwordButton->OnButtonReleased.AddDynamic(this, &AKnight::SwordButtonReleased);
			SwordButton->OnButtonDraggedOff.AddDynamic(this, &AKnight::SwordButtonDraggedOff);
		}
	}
}

// Called every frame
void AKnight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bButtonHeld && bSwordInteractable)
	{
		SwordButtonHeldTime += DeltaTime;
		FMath::Clamp(SwordButtonHeldTime, 0,5);

		SwordScreenShake(SwordButtonHeldTime);
		if (SwordButtonHeldTime > 3)
		{
			UE_LOG(LogTemp, Warning, TEXT("sword held for 3 seconds"));
			ServerChangeSwordState();
		}
	}
	else
	{
		SwordButtonHeldTime = 0;
	}
}

void AKnight::ChangeSwordState()
{
	bSwordInserted = !bSwordInserted;
	SwordButtonHeldTime = 0;
	GetWorld()->GetTimerManager().SetTimer(SwordTimeoutTimerHandle, this, &AKnight::EndSwordInteractabilityTimeout,15,false);

	if (IsValid(SwordMesh))
	{
		USwordAnimInstance* AnimInstance = Cast<USwordAnimInstance>(SwordMesh->GetAnimInstance());
		if (AnimInstance)
		{
			AnimInstance->bIsInserted = bSwordInserted;
			AnimInstance->bIsHeld = false;
		}
	}
	if (!bSwordInserted)
	{
		//game mode is watching this
		bIsAwake = true;
	}
	if (bSwordInserted)
	{
		ResetKnight();
	}
}

void AKnight::EndSwordInteractabilityTimeout()
{
	bSwordInteractable = true;
}

void AKnight::ServerChangeSwordState_Implementation()
{
	if (!bSwordInteractable) return;
	bSwordInteractable = false;
	MulticastChangeSwordState();
}

void AKnight::MulticastChangeSwordState_Implementation()
{
	ChangeSwordState();
}

void AKnight::SetKnightAnimationTime(float MatchTime)
{
	UE_LOG(LogTemp, Warning, TEXT("knight animation time set"));
	if (IsValid(KnightMesh))
	{
		UKnightAnimInstance* AnimInstance = Cast<UKnightAnimInstance>(KnightMesh->GetAnimInstance());
		if (AnimInstance)
		{
			AnimInstance->ActivateKnight(MatchTime);
		}
	}
	if (IsValid(SwordMesh))
	{
		USwordAnimInstance* SwordAnimInstance = Cast<USwordAnimInstance>(SwordMesh->GetAnimInstance());
		if (SwordAnimInstance)
		{
			SwordAnimInstance->ActivateSword(MatchTime);
		}
	}
}

void AKnight::TellSwordItsHeld()
{
	if (IsValid(SwordMesh))
	{
		USwordAnimInstance* SwordAnimInstance = Cast<USwordAnimInstance>(SwordMesh->GetAnimInstance());
		if (SwordAnimInstance)
		{
			SwordAnimInstance->bIsHeld = true;
		}
	}
}

void AKnight::ResetKnight()
{
	//todo implement knight reset logic
	bIsAwake = false;
}
