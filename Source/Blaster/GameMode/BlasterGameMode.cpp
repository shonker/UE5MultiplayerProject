// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Blaster/BlasterPlayerState/BlasterPlayerState.h"
#include "Blaster/ProceduralEnvironments/ProcNeighborhood.h"
#include "EngineUtils.h"
#include "GameFramework/GameSession.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/HomeBase/HomeBase.h"
#include "Blaster/HomeBase/Mama.h"

namespace MatchState 
{
	const FName Cooldown = FName("Cooldown");
	const FName Judgement = FName("Judgement");
	const FName GameOver = FName("GameOver");
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
}

void ABlasterGameMode::InitGameState()
{
	Super::InitGameState();

	for (AProcNeighborhood* Spawner : TActorRange<AProcNeighborhood>(GetWorld()))
	{
		ProcNeighborhood = Spawner;
		break;
	}

	if (ProcNeighborhood == nullptr) return;

	RandomSeed = FMath::Rand();

	ProcNeighborhood->ProcGen(RandomSeed);

}

bool ABlasterGameMode::ReadyToStartMatch_Implementation()
{
	// Check if clients have finished procgen: We delay begin play of world Actors until clients have finished procgen
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (ABlasterPlayerController* PC = Cast<ABlasterPlayerController>(Iterator->Get()))
		{
			if (!PC->bClientFinishedProceduralGeneration)
			{
				return false;
			}
		}
	}

	// If bDelayed Start is set, wait for a manual match start
	if (bDelayedStart)
	{
		return false;
	}

	// By default start when we have > 0 players
	if (GetMatchState() == MatchState::WaitingToStart)
	{
		if (NumPlayers + NumBots > 0)
		{
			return true;
		}
	}
	return false;
}

void ABlasterGameMode::HandleMatchIsWaitingToStart()
{
	if (GameSession != nullptr)
	{
		GameSession->HandleMatchIsWaitingToStart();
	}
}


void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();

	for (TActorIterator<AHomeBase> It(GetWorld()); It; ++It)
	{
		HomeBase = *It;
		break;
	}
	for (TActorIterator<AMama> It(GetWorld()); It; ++It)
	{
		Mama = *It;
		break;
	}
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	//loop through all player controllers to call OnMatchStateSet
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		//dereference iterator
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if (BlasterPlayer)
		{
			BlasterPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void ABlasterGameMode::StartCountdown()
{
	if (GetMatchState() == MatchState::WaitingToStart)
	{
		StartMatch(); // This will change the state to InProgress and start the countdown.
	}
}

void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
		
	//cant be switch case ????
	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;

		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
			if (CountdownTime < 0.f)
			{
				if (HomeBase)
				{
					HomeBase->BeginJudgement();
				}
				SetMatchState(MatchState::Judgement);
			}
	}
	//consider if this setmatchstate can be simply callable from mama's state change
	//instead of being checked here every tick lol
	else if (MatchState == MatchState::Judgement)
	{
		if (Mama) 
		{
			if (Mama->GetState() == EState::WIN)
			{
				SetMatchState(MatchState::Cooldown);
			}
			else if (Mama->GetState() == EState::LOSS)
			{
				SetMatchState(MatchState::GameOver);
			}
		}
	}

	else if (MatchState == MatchState::Cooldown)
	{
		//must reactivate seamless travel when building I think
		RestartGame();
	}
	else if (MatchState == MatchState::GameOver)
	{
		
	}
}

void ABlasterGameMode::PlayerEliminated(
	class ABlasterCharacter* ElimmedCharacter,
	class ABlasterPlayerController* VictimController,
	class ABlasterPlayerController* AttackerController)
{
	ABlasterPlayerState* AttackerPlayerState =
		AttackerController ?
		Cast<ABlasterPlayerState>(AttackerController->PlayerState)
		: nullptr;
	ABlasterPlayerState* VictimPlayerState =
		VictimController ?
		Cast<ABlasterPlayerState>(VictimController->PlayerState)
		: nullptr;

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlasterGameState)
	{
		AttackerPlayerState->AddToScore(6.66f);
		BlasterGameState->UpdateTopScore(AttackerPlayerState);
	}
	
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDebt(666);
	}

	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}

void ABlasterGameMode::RequestRespawn(
	ACharacter* ElimmedCharacter, 
	AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}
