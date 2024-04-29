// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDebt(float Debt);
	void SetHUDInteractText(const FString& Interaction);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdownText(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	/*
		proc gen optimization
	*/
	UFUNCTION(Server, Reliable)
	void ServerClientFinishedProcGen();
	UFUNCTION(Server, Reliable)
	void ServerRequestProcGenData();
	UFUNCTION(Client, Reliable)
	void ClientReceiveProcGenData(uint32 randomSeed);
	/*
	
	*/
	void PostNetInit();
	void PostInitializeComponents();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void CheckTimeSync(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	virtual float GetServerTime(); //synced w/ server world clock
	virtual void ReceivedPlayer() override;//sync w/ server clock asap

	void OnMatchStateSet(FName State);
	void HandleMatchHasStarted();
	void HandleCooldown();

	bool bClientFinishedProceduralGeneration = false;
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();
	/*
	* Sync time between the client and server
	*/

	//Requests the current server time, pasing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	//reports the current server time to the client in response to ServerRequestServerTime 
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f; //diff bet client and server time
	
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
		void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);

private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	bool bInitializeCharacterOverlay = false;

	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	float HUDDebt;

	FTimerHandle BlinkTimer;
	UFUNCTION()
	void BlinkTimerFinished();
	UFUNCTION()
	void TimeRunningOut();
	
	FString GetUserAssignedInputFor(const FName ActionName);

public:
	FORCEINLINE FName GetMatchState() const {return MatchState; }
}; 
