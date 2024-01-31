// Fill out your copyright notice in the Description page of Project Settings.

#include "RagdollCube.h"
#include "Components/BoxComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ARagdollCube::ARagdollCube()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PhysicsBox = CreateDefaultSubobject<UBoxComponent>(TEXT("PhysicsBox"));
	SetRootComponent(PhysicsBox);
	PhysicsBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PhysicsBox->SetVisibility(true);
	PhysicsBox->SetSimulatePhysics(false);
	PhysicsBox->SetCollisionResponseToAllChannels(ECR_Overlap);
	PhysicsBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	PhysicsBox->SetIsReplicated(true);

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));

	bReplicates = true;
}

// Called when the game starts or when spawned
void ARagdollCube::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		PhysicsBox->OnComponentBeginOverlap.AddDynamic(this, &ARagdollCube::OnPhysicsBoxOverlap);
		PhysicsBox->OnComponentEndOverlap.AddDynamic(this, &ARagdollCube::OnPhysicsBoxEndOverlap);
	}
	GrabBodyAtSocket();
}

// Called every frame
void ARagdollCube::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		if (bFollowPlayer)
		{
			if (FollowChar)
			{
				FVector CharLocation = FollowChar->GetActorLocation();
				FVector DirectionToChar = (GetActorLocation() - CharLocation).GetSafeNormal();
				FVector TeleportLocation = CharLocation + DirectionToChar * 100.f; // 100 units away from character

				SetActorLocation(TeleportLocation, true, nullptr, ETeleportType::TeleportPhysics);
			}
		}
		else
		{
			if (CharacterMesh)
			{
				FVector CharLocation = CharacterMesh->GetSocketLocation(TEXT("spine_001"));
				SetActorLocation(CharLocation, true, nullptr, ETeleportType::TeleportPhysics);
			}
		}
	}

	if (PhysicsHandle && PhysicsHandle->GrabbedComponent)
	{
		// Update to the desired target location
		FVector NewTargetLocation = GetActorLocation() /* Calculate or get the new target location */;
		FRotator NewTargetRotation = GetActorRotation()/* Calculate or get the new target rotation */;

		PhysicsHandle->SetTargetLocationAndRotation(NewTargetLocation, NewTargetRotation);
	}
}

void ARagdollCube::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(ARagdollCube, CharacterMesh);
}
void ARagdollCube::GrabBodyAtSocket()
{
	if (CharacterMesh && PhysicsHandle)
	{
		CharacterMesh->WakeAllRigidBodies();
		UE_LOG(LogTemp, Log, TEXT("grabb"));

		FName SocketName = TEXT("spine");
		FVector SocketLocation = CharacterMesh->GetSocketLocation(SocketName);

		// Find the bone name associated with the socket
		FName BoneName = CharacterMesh->GetSocketBoneName(SocketName);
		UE_LOG(LogTemp, Warning, TEXT("Bone Name: %s"), *BoneName.ToString());

		// Grab the component (the skeletal mesh) at the location of the socket
		PhysicsHandle->GrabComponentAtLocationWithRotation(
			CharacterMesh,
			BoneName,
			SocketLocation,
			FRotator::ZeroRotator // Or the desired rotation
		);
	}
}
void ARagdollCube::OnPhysicsBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherComp && OtherComp->GetName() == FString("InteractSphere"))
	{
		UE_LOG(LogTemp, Log, TEXT("overlap detected"));
		ABlasterCharacter* BChar = Cast<ABlasterCharacter>(OtherActor);
		if (BChar)
		{
			BChar->OverlappedBody = this;
		}
	}
}

void ARagdollCube::OnPhysicsBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherComp && OtherComp->GetName() == FString("InteractSphere"))
	{
		ABlasterCharacter* BChar = Cast<ABlasterCharacter>(OtherActor);
		if (BChar)
		{
			BChar->OverlappedBody = nullptr;
		}
	}
}
void ARagdollCube::SetCharacterMesh(USkeletalMeshComponent* Mesh)
{
	CharacterMesh = Mesh;
}

