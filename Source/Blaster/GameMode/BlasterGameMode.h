// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

namespace MatchState
{
	extern BLASTER_API const FName PreDream; 
	extern BLASTER_API const FName Dreaming; 
	extern BLASTER_API const FName Judgement; 
	extern BLASTER_API const FName GameOver;
	extern BLASTER_API const FName Cooldown; //Match duration has been reached. Display winner and begin cooldown timer
}

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	void StartCountdown();
	ABlasterGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(
		ABlasterCharacter* ElimmedCharacter,
		ABlasterPlayerController* VictimController,
		AController* AttackerController
	);
	virtual void RequestRespawn(
		class ACharacter* ElimmedCharacter,
		AController* ElimmedController
	);
	UPROPERTY()
	class AHomeBase* HomeBase;
	UPROPERTY()
	class AMama* Mama;
	UPROPERTY()
	class AKnight* Knight;
	
	/*
		Best not to confuse yourself, and
		only set these "time" vars in editor!
	*/
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 2.f;	
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	float MatchTime= 10.f;
		
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	float JudgementTime= 40.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CooldownTime= 2.f;

	float LevelStartingTime = 0.f;

	UPROPERTY()
	class AProcNeighborhood* ProcNeighborhood;

	UPROPERTY()
	uint32 RandomSeed;
protected:
	virtual void BeginPlay() override;
	virtual void InitGameState() override;
	virtual bool ReadyToStartMatch_Implementation() override;
	virtual void HandleMatchIsWaitingToStart() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;
public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FName GetMatchStateVirtual() const { return MatchState; }
};
