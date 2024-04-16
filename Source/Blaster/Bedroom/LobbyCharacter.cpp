// LobbyCharacter.cpp

#include "LobbyCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h" 

ALobbyCharacter::ALobbyCharacter()
{
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
}

void ALobbyCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ALobbyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALobbyCharacter::OnRep_CharacterMeshEnum(EBedCharMesh NewCharMeshEnum)
{
	SetCharacterMeshEnum(CharacterMeshEnum);
}

void ALobbyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyCharacter, bAsleep);
	DOREPLIFETIME(ALobbyCharacter, CharacterMeshEnum);
	DOREPLIFETIME(ALobbyCharacter, CharacterLocation);

}

void ALobbyCharacter::SelectCharacterMeshAndLocation(const TArray<ALobbyCharacter*>& ExistingCharacters)
{
	TArray<ECharacterLocation> AvailableLocations = {
		ECharacterLocation::COUCH, ECharacterLocation::BEANBAG, ECharacterLocation::BED, ECharacterLocation::PILLOW, ECharacterLocation::WINDOW
	};
	TArray<EBedCharMesh> AvailableMeshes = {
		EBedCharMesh::RIBBIT,EBedCharMesh::RABBIT,EBedCharMesh::RJ,EBedCharMesh::SCENE,EBedCharMesh::SCENE2
	};

	for (const auto& Char : ExistingCharacters)
	{
			AvailableLocations.Remove(Char->CharacterLocation);
			AvailableMeshes.Remove(Char->CharacterMeshEnum);
	}

	if (AvailableLocations.Num() > 0)
	{
		int32 LocationIndex = FMath::RandRange(0, AvailableLocations.Num() - 1);
		CharacterLocation = AvailableLocations[LocationIndex];
	}
	else
	{
		CharacterLocation = ECharacterLocation::COUCH;
	}

	if (AvailableMeshes.Num() > 0)
	{
		int32 MeshIndex = FMath::RandRange(0, AvailableLocations.Num() - 1);
		CharacterMeshEnum = AvailableMeshes[MeshIndex];
	}

	SetCharacterMeshEnum(CharacterMeshEnum);
}

void ALobbyCharacter::SetCharacterMeshEnum(EBedCharMesh BedMeshEnum)
{
	switch (BedMeshEnum)
	{
	case EBedCharMesh::RJ:
		MeshComponent->SetSkeletalMesh(RJMesh);
		break;
	case EBedCharMesh::RIBBIT:
		MeshComponent->SetSkeletalMesh(RibbitMesh);
		break;
	case EBedCharMesh::RABBIT:
		MeshComponent->SetSkeletalMesh(RabbitMesh);
		break;
	case EBedCharMesh::SCENE:
		MeshComponent->SetSkeletalMesh(SceneMesh);
		break;
	case EBedCharMesh::SCENE2:
		MeshComponent->SetSkeletalMesh(SceneMesh);
		break;
	}
}
