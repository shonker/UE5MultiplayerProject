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

void ALobbyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyCharacter, bAsleep);
	DOREPLIFETIME(ALobbyCharacter, MeshComponent);
	DOREPLIFETIME(ALobbyCharacter, CharacterLocation);

}

void ALobbyCharacter::SelectCharacterMeshAndLocation(const TArray<ALobbyCharacter*>& ExistingCharacters)
{
	TArray<ECharacterLocation> AvailableLocations = { ECharacterLocation::COUCH, ECharacterLocation::BEANBAG, ECharacterLocation::BED, ECharacterLocation::PILLOW, ECharacterLocation::WINDOW };

	for (const auto& Char : ExistingCharacters)
	{
		AvailableLocations.Remove(Char->CharacterLocation);
	}

	if (AvailableLocations.Num() > 0)
	{
		int32 Index = FMath::RandRange(0, AvailableLocations.Num() - 1);
		CharacterLocation = AvailableLocations[Index];
	}
	else
	{
		CharacterLocation = ECharacterLocation::COUCH;
	}

	if (SkeletalMeshOptions.Num() > 0)
	{
		int32 MeshIndex = FMath::RandRange(0, SkeletalMeshOptions.Num() - 1);
		MeshComponent->SetSkeletalMesh(SkeletalMeshOptions[MeshIndex]);
	}

}
