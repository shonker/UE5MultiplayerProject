// Bedroom.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Bedroom.generated.h"

UCLASS()
class BLASTER_API ABedroom : public AActor
{
	GENERATED_BODY()

public:
	ABedroom();

protected:
	virtual void BeginPlay() override;

public:
	void SpawnLobbyCharacter(APlayerController* NewPlayer);
	UPROPERTY(BlueprintReadWrite)
	 TArray<class ALobbyCharacter*> LobbyCharacters;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
		TSubclassOf<ALobbyCharacter> LobbyCharacterBP;

	UFUNCTION(BlueprintCallable, Category = "Effects")
		void Shake();

};
