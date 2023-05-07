// Fill out your copyright notice in the Description page of Project Settings.


#include "Limb.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ALimb::ALimb()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	LimbMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Limb"));
	//SetRootComponent(LimbMesh);

	LimbMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	LimbMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	LimbMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	LimbMesh->SetSimulatePhysics(true);
	LimbMesh->SetEnableGravity(true);
	LimbMesh->SetNotifyRigidBodyCollision(true);
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	//CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->SetupAttachment(LimbMesh);

	CameraBoom->TargetArmLength = 350.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

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

	UE_LOG(LogTemp, Display, TEXT("inputs being bound"));
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

void ALimb::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

void ALimb::MoveForward(float Value)
{
	if (Value == 0) return;
	FVector AimDirection = FollowCamera->GetForwardVector();
	ServerMoveForward(Value, AimDirection);
}

void ALimb::ServerMoveForward_Implementation(float Value, FVector_NetQuantize10 AimDirection)
{
	MulticastMoveForward(Value, AimDirection);	
}

void ALimb::MulticastMoveForward_Implementation(float Value, FVector_NetQuantize10 AimDirection)
{
	if (!LimbMesh) return;
	LimbMesh->AddImpulse(AimDirection * Value * 170);
}



void ALimb::MoveRight(float Value)
{
	if (Value == 0) return;
	ServerMoveRight(Value);
}

void ALimb::ServerMoveRight_Implementation(float Value)
{
	MulticastMoveRight(Value);
}

void ALimb::MulticastMoveRight_Implementation(float Value)
{
	if (!LimbMesh) return;
	LimbMesh->AddTorqueInRadians(FVector(0.f,0.f,50000.f)*Value);
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
	ServerJump();	
}

void ALimb::ServerJump_Implementation()
{
	MulticastJump();
}

void ALimb::MulticastJump_Implementation()
{
	FVector UpImpulse = FVector(0.f,0.f,5000);
	if (!LimbMesh) return;
	LimbMesh->AddImpulse(UpImpulse);
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