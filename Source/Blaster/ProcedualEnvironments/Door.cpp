// Fill out your copyright notice in the Description page of Project Settings.


#include "Door.h"


// Sets default values
ADoor::ADoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	DoorMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Door Mesh"));
	SetRootComponent(DoorMesh);
	DoorMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	//LockButton = CreateDefaultSubobject<AMyButton>(TEXT("Door Lock"));

	DoorKnobButtonComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("Door Knob Button"));
	DoorKnobButtonComponent->SetupAttachment(DoorMesh, FName("DoorKnobsSocket"));
	DoorKnobButtonComponent->SetChildActorClass(AMyButton::StaticClass());

}

// Called when the game starts or when spawned
void ADoor::BeginPlay()
{
	Super::BeginPlay();	

	/*if (GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		DoorKnobButton = GetWorld()->SpawnActor<AMyButton>(AMyButton::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (DoorKnobButton)
		{
			const FName SocketName = FName("DoorKnobsSocket");
			DoorKnobButton->AttachToComponent(DoorMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
		}
	}*/

	if (DoorKnobButtonComponent)
	{
		AMyButton* DoorKnobButton = Cast<AMyButton>(DoorKnobButtonComponent->GetChildActor());
		if (DoorKnobButton)
		{
			DoorKnobButton->OnButtonPressed.AddDynamic(this, &ADoor::KnobButtonPress);
			DoorKnobButton->OnButtonReleased.AddDynamic(this, &ADoor::KnobButtonRelease);
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

	/*DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, InteractTargetLocation, COND_SkipOwner);
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);*/
}


void ADoor::KnobButtonPress()
{
	UE_LOG(LogTemp, Log, TEXT("I am turn knob"));
}

void ADoor::KnobButtonRelease()
{
	IsOpen = !IsOpen;
	if (IsOpen)
	{
		UE_LOG(LogTemp, Log, TEXT("I am opened"));
	}
	if (!IsOpen)
	{
		UE_LOG(LogTemp, Log, TEXT("I am closed"));
	}
}