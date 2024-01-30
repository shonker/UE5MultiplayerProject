// Fill out your copyright notice in the Description page of Project Settings.

#include "RagdollCube.h"
#include "Components/BoxComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

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
	PhysicsBox->bReplicatePhysicsToAutonomousProxy = false;
	if (HasAuthority())
	{
		PhysicsBox->SetSimulatePhysics(true);
		PhysicsBox->bReplicatePhysicsToAutonomousProxy = true;
	}
	PhysicsBox->SetCollisionResponseToAllChannels(ECR_Block);
	PhysicsBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
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
}

// Called every frame
void ARagdollCube::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bFollowPlayer && HasAuthority())
	{
		if (FollowChar)
		{
			FVector CharLocation = FollowChar->GetActorLocation();
			FVector DirectionToChar = (GetActorLocation() - CharLocation).GetSafeNormal();
			FVector TeleportLocation = CharLocation + DirectionToChar * 100.f; // 100 units away from character

			SetActorLocation(TeleportLocation, true, nullptr, ETeleportType::TeleportPhysics);
		}
	}
	
	if (CharacterMesh)
	{
		CharacterMesh->SetWorldLocationAndRotation(GetActorLocation(), GetActorRotation(), false, nullptr, ETeleportType::TeleportPhysics);
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

