// Fill out your copyright notice in the Description page of Project Settings.


#include "Door.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ADoor::ADoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	DoorMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Door Mesh"));
	SetRootComponent(DoorMesh);
	DoorMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

		DoorKnobButtonComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("Door Knob Button"));
		DoorKnobButtonComponent->SetupAttachment(DoorMesh, FName("DoorKnobsSocket"));
		DoorKnobButtonComponent->SetChildActorClass(AMyButton::StaticClass());
		DoorKnobButtonComponent->SetIsReplicated(true);
		
		LockButton = CreateDefaultSubobject<UChildActorComponent>(TEXT("Slide Lock Button"));
		LockButton->SetupAttachment(DoorMesh, FName("LockSocket"));
		LockButton->SetChildActorClass(AMyButton::StaticClass());
		LockButton->SetIsReplicated(true);

	/*	LockButtonComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("Lock Button"));
		LockButtonComponent->SetupAttachment(DoorMesh, FName("LockSocket"));
		LockButtonComponent->SetChildActorClass(AMyButton::StaticClass());
		LockButtonComponent->SetIsReplicated(true);*/

}

// Called when the game starts or when spawned
void ADoor::BeginPlay()
{
	Super::BeginPlay();	
	if (LockButton && LockButton->GetChildActor())
	{
		AMyButton* SlideLock = Cast<AMyButton>(LockButton->GetChildActor());
		if (SlideLock)
		{
			SlideLock->OnButtonPressed.AddDynamic(this, &ADoor::LockButtonPress);
			SlideLock->InteractionText = TEXT("LOCK");
		}
	}
	if (DoorKnobButtonComponent && DoorKnobButtonComponent->GetChildActor())
	{
		AMyButton* DoorKnobButton = Cast<AMyButton>(DoorKnobButtonComponent->GetChildActor());
		if (DoorKnobButton)
		{
			DoorKnobButton->OnButtonPressed.AddDynamic(this, &ADoor::KnobButtonPress);
			DoorKnobButton->OnButtonReleased.AddDynamic(this, &ADoor::KnobButtonRelease);
			DoorKnobButton->OnButtonDraggedOff.AddDynamic(this, &ADoor::KnobButtonDraggedOff);
			DoorKnobButton->InteractionText = TEXT("OPEN");
		}
	}
}

// Called every frame
void ADoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ADoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADoor, bKnobTurning);
	DOREPLIFETIME(ADoor, bAttemptOpen);
	DOREPLIFETIME(ADoor, bIsOpen);
	DOREPLIFETIME(ADoor, bIsLocked);
}


void ADoor::KnobButtonPress()
{
	bKnobTurning = true;
	if (bIsLocked)
	{
		bAttemptOpen = true;
	}
}

void ADoor::KnobButtonRelease()
{
	bKnobTurning = false;

	if (!bIsLocked)
	{
		bIsOpen = !bIsOpen;
	}
	bAttemptOpen = false;
}

void ADoor::KnobButtonDraggedOff()
{
	bKnobTurning = false;
	bAttemptOpen = false;
}

void ADoor::LockButtonPress()
{
	bIsLocked = !bIsLocked;
}
