// LobbyCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyCharacter.generated.h"

UENUM(BlueprintType)
enum class ECharacterLocation : uint8
{
	COUCH,
	BEANBAG,
	BED,
	PILLOW,
	WINDOW
};

UCLASS()
class BLASTER_API ALobbyCharacter : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALobbyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", Replicated)
		USkeletalMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Options")
		TArray<USkeletalMesh*> SkeletalMeshOptions;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Replicated)
		ECharacterLocation CharacterLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Replicated)
		bool bAsleep = false;

	void SelectCharacterMeshAndLocation(const TArray<ALobbyCharacter*>& ExistingCharacters);

private:
	APlayerController* OwningPlayerController;

public:
	void SetOwningPlayer(APlayerController* PlayerController) { OwningPlayerController = PlayerController; }
	APlayerController* GetOwningPlayer() const { return OwningPlayerController; }
};
