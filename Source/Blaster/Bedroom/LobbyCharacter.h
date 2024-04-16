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

UENUM(BlueprintType)
enum class EBedCharMesh : uint8
{
	RIBBIT,
	RABBIT,
	RJ,
	SCENE,
	SCENE2
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComponent;

	UFUNCTION()
	void OnRep_CharacterMeshEnum(EBedCharMesh NewCharMeshEnum);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_CharacterMeshEnum)
	EBedCharMesh CharacterMeshEnum;

	UPROPERTY(EditAnywhere, Category = "Playable Characters")
	USkeletalMesh* RabbitMesh;
	UPROPERTY(EditAnywhere, Category = "Playable Characters")
	USkeletalMesh* RJMesh;
	UPROPERTY(EditAnywhere, Category = "Playable Characters")
	USkeletalMesh* RibbitMesh;
	UPROPERTY(EditAnywhere, Category = "Playable Characters")
	USkeletalMesh* SceneMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Replicated)
		ECharacterLocation CharacterLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Replicated)
		bool bAsleep = false;

	void SelectCharacterMeshAndLocation(const TArray<ALobbyCharacter*>& ExistingCharacters);

private:
	UPROPERTY()
	APlayerController* OwningPlayerController;

public:
	void SetOwningPlayer(APlayerController* PlayerController) { OwningPlayerController = PlayerController; }
	APlayerController* GetOwningPlayer() const { return OwningPlayerController; }
	void SetCharacterMeshEnum(EBedCharMesh BedMeshEnum);
};
