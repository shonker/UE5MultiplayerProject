// Fill out your copyright notice in the Description page of Project Settings.


#include "Limb.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"




// Sets default values
ALimb::ALimb()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
	// CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	// CameraBoom->SetupAttachment(GetRoot());
	// CameraBoom->TargetArmLength = 350.f;
	// CameraBoom->bUsePawnControlRotation = true;

	// FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	// FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// FollowCamera->bUsePawnControlRotation = false;

}

// Called when the game starts or when spawned
void ALimb::BeginPlay()
{
	Super::BeginPlay();
	this->SetReplicates(true);
}

// Called every frame
void ALimb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority()) return;

	
}

// Called to bind functionality to input
void ALimb::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//apply vertical impulse to physics body
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ALimb::Jump);

	//apply horizontal impulses to physics body
	PlayerInputComponent->BindAxis("MoveForward", this, &ALimb::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ALimb::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ALimb::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ALimb::LookUp);

	//connect limb to interface
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ALimb::EquipButtonPressed);

	//apply strong forward impulse to physics body
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ALimb::FireButtonPressed);
}


void ALimb::MoveForward(float Value)
{
	if (!IsLocallyControlled()) return;

	FVector LookDirection = FollowCamera->GetForwardVector();
	FVector FwdImpulse = LookDirection * Value * 10;

	// if (Controller != nullptr && Value != 0.f)
	// {
	// 	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	// 	//Return an f vector only containing the rotation on the x axis, zero'd out on the pitch and roll
	// 	const FVector Direction( FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
	// 	//this only tells the system that movement input is applied
	// 	//speed and direction belong in the char movement component
	// 	AddMovementInput(Direction, Value);
	// }
}

void ALimb::MoveRight(float Value)
{
	// if (Controller != nullptr && Value != 0.f)
	// {
	// 	//same as move forward, BUT isolate Y axis
	// 	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	// 	const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
	// 	AddMovementInput(Direction, Value);
	// }
}

void ALimb::Turn(float Value)
{
	AddControllerYawInput(Value);
	
}

void ALimb::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ALimb::Jump()
{
	
}

void ALimb::FireButtonPressed()
{
}

void ALimb::EquipButtonPressed()
{
	// if (Combat)
	// {
	// 	if (HasAuthority()) //are we server?
	// 	{
	// 	Combat->EquipWeapon(OverlappingWeapon);
	// 	}
	// 	else
	// 	{
	// 		ServerEquipButtonPressed();
	// 	}
	// }
}