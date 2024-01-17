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
		
		DoorKnobButtonComponent2 = CreateDefaultSubobject<UChildActorComponent>(TEXT("Door Knob Button2"));
		DoorKnobButtonComponent2->SetupAttachment(DoorMesh, FName("DoorKnobsSocket"));
		DoorKnobButtonComponent2->SetChildActorClass(AMyButton::StaticClass());
		DoorKnobButtonComponent2->SetIsReplicated(true);

		LockButtonComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("Lock Button"));
		LockButtonComponent->SetupAttachment(DoorMesh, FName("LockSocket"));
		LockButtonComponent->SetChildActorClass(AMyButton::StaticClass());
		LockButtonComponent->SetIsReplicated(true);

}

// Called when the game starts or when spawned
void ADoor::BeginPlay()
{
	Super::BeginPlay();	
	if (DoorKnobButtonComponent2)
	{
		AMyButton* DoorKnobButton2 = Cast<AMyButton>(DoorKnobButtonComponent2->GetChildActor());
		if (DoorKnobButton2)
		{
			DoorKnobButton2->OnButtonPressed.AddDynamic(this, &ADoor::LockButtonPress);
		}
		else
		{
			if (HasAuthority()) UE_LOG(LogTemp, Warning, TEXT("cast failed for server"));
		}
	}
	if (DoorKnobButtonComponent)
	{
		AMyButton* DoorKnobButton = Cast<AMyButton>(DoorKnobButtonComponent->GetChildActor());
		if (DoorKnobButton)
		{
			DoorKnobButton->OnButtonPressed.AddDynamic(this, &ADoor::KnobButtonPress);
			DoorKnobButton->OnButtonReleased.AddDynamic(this, &ADoor::KnobButtonRelease);
			DoorKnobButton->OnButtonDraggedOff.AddDynamic(this, &ADoor::KnobButtonDraggedOff);
		}
	}
}

void ADoor::AttemptLockButtonCast()
{
	if (LockButtonComponent)
	{
		AMyButton* LockButton = Cast<AMyButton>(LockButtonComponent->GetChildActor());
		if (LockButton)
		{
			if (!HasAuthority()) UE_LOG(LogTemp, Log, TEXT("succesfully cast door lock"));

			LockButton->OnButtonPressed.AddDynamic(this, &ADoor::LockButtonPress);
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_AttemptCast);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Lock cast failed, will reattempt in 10 seconds"));
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_AttemptCast, this, &ADoor::AttemptLockButtonCast, 2.0f, false);
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
	if (HasAuthority())	UE_LOG(LogTemp, Log, TEXT("as server:"));
	if (!HasAuthority())	UE_LOG(LogTemp, Log, TEXT("as not server:"));
	if (bIsLocked)
	{
		UE_LOG(LogTemp, Log, TEXT("I was locked"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("I was NOT locked"));
	}
	bIsLocked = !bIsLocked;
	if (bIsLocked)
	{
		UE_LOG(LogTemp, Log, TEXT("Now i am locked"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Now I'm NOT locked"));
	}
}
