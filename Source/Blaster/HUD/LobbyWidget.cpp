// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyWidget.h"
#include "Components/Button.h"
#include "Net/UnrealNetwork.h" 
#include "Blaster/GameMode/LobbyGameMode.h"
#include "Engine/Engine.h"
#include "Blaster/PlayerController/LobbyPlayerController.h"

void ULobbyWidget::LobbySetup(FString MenuPath)
{
	PathToMenu = MenuPath;
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	APlayerController* OwningPlayer = GetOwningPlayer();
	if (OwningPlayer)
	{
		FInputModeUIOnly InputModeData;
		InputModeData.SetWidgetToFocus(TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		OwningPlayer->SetInputMode(InputModeData);
		OwningPlayer->SetShowMouseCursor(true);
	}
}

bool ULobbyWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &ThisClass::StartButtonClicked);
	}

	if (LeaveButton)
	{
		LeaveButton->OnClicked.AddDynamic(this, &ThisClass::LeaveButtonClicked);
	}

	LobbyGameMode = Cast<ALobbyGameMode>(GetWorld()->GetAuthGameMode());

	APlayerController* OwningPlayer = GetOwningPlayer();
	if (OwningPlayer)
	{
		Cast<ALobbyPlayerController>(OwningPlayer)->SetLobbyWidget(this);
	}

	return true;
}

void ULobbyWidget::StartButtonClicked()
{
	LobbyGameMode = nullptr ? Cast<ALobbyGameMode>(GetWorld()->GetAuthGameMode()) : LobbyGameMode;

	if (LobbyGameMode) //this will only ever pass on the server
	{
		LobbyGameMode->StartGame();
	}
	MenuTearDown();
}

void ULobbyWidget::LeaveButtonClicked()
{
	UWorld* World = GetWorld();
	if (!World) return;
	
	APlayerController* OwningPlayer = GetOwningPlayer();
	if (!OwningPlayer) return;

	if (OwningPlayer->HasAuthority()) // This check is for the server/host
	{
		// For the host, notify all clients to disconnect, then travel to the main menu
		// Depending on your game's architecture, you might use a custom event or RPC to notify clients

		// This command tells the server to switch levels, effectively "kicking" all clients as they'll lose connection
		FString Command = FString::Printf(TEXT("ServerTravel %s"), *PathToMenu);
		World->Exec(World, *Command);
	}
	else // For clients
	{
		// This command makes the client travel to the specified level (main menu)
		FString Command = FString::Printf(TEXT("Open %s"), *PathToMenu);
		OwningPlayer->ClientTravel(*Command, ETravelType::TRAVEL_Absolute);
	}

	MenuTearDown();
}

void ULobbyWidget::MenuTearDown()
{
	RemoveFromViewport();
	APlayerController* OwningPlayer = GetOwningPlayer();
	if (OwningPlayer)
	{
		FInputModeGameOnly InputModeData;
		OwningPlayer->SetInputMode(InputModeData);
		OwningPlayer->bShowMouseCursor = false;
	}
}
