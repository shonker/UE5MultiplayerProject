// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyWidget.h"

#include "OnlineSubsystem.h"
#include "Components/Button.h"
#include "Net/UnrealNetwork.h" 
#include "Blaster/GameMode/LobbyGameMode.h"
#include "Engine/Engine.h"
#include "Blaster/PlayerController/LobbyPlayerController.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"

void ULobbyWidget::LobbySetup(FString MenuPath)
{
	PathToMenu = MenuPath;
	// AddToViewport();
	// SetVisibility(ESlateVisibility::Visible);
	// bIsFocusable = true;
	//
	// APlayerController* OwningPlayer = GetOwningPlayer();
	// if (OwningPlayer)
	// {
	// 	FInputModeUIOnly InputModeData;
	// 	InputModeData.SetWidgetToFocus(TakeWidget());
	// 	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	// 	OwningPlayer->SetInputMode(InputModeData);
	// 	OwningPlayer->SetShowMouseCursor(true);
	// }
}

bool ULobbyWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}
	
	LobbyGameMode = Cast<ALobbyGameMode>(GetWorld()->GetAuthGameMode());

	APlayerController* OwningPlayer = GetOwningPlayer();
	if (OwningPlayer)
	{
		Cast<ALobbyPlayerController>(OwningPlayer)->SetLobbyWidget(this);
	}
	
	if (StartButton)
	{
		if (OwningPlayer && OwningPlayer->HasAuthority())
		{
			StartButton->OnClicked.AddDynamic(this, &ThisClass::StartButtonClicked);
		}
		else
		{
			StartButton->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (LeaveButton)
	{
		LeaveButton->OnClicked.AddDynamic(this, &ThisClass::LeaveButtonClicked);
	}

	return true;
}

void ULobbyWidget::StartButtonClicked()
{
	LobbyGameMode = nullptr ? Cast<ALobbyGameMode>(GetWorld()->GetAuthGameMode()) : LobbyGameMode;

	if (LobbyGameMode) //this will only ever pass on the server
	{
		LobbyGameMode->StartGame();
		//MenuTearDown();
	}
}

void ULobbyWidget::LeaveButtonClicked()
{
	UWorld* World = GetWorld();
	if (!World) return;
	
	APlayerController* OwningPlayer = GetOwningPlayer();
	if (!OwningPlayer) return;

	if (OwningPlayer->HasAuthority())
	{
		// Assuming you want to end the session for the host and return to the menu
		IOnlineSessionPtr Sessions = IOnlineSubsystem::Get()->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->DestroySession(NAME_GameSession, FOnDestroySessionCompleteDelegate::CreateUObject(this, &ULobbyWidget::OnSessionDestroyed));
		}
	}
	else 
	{
		// Clients simply travel back to the menu map
		ClientReturnToMenu();
	}
}

void ULobbyWidget::OnSessionDestroyed(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		// After successfully destroying the session, the host can travel back to the menu
		ClientReturnToMenu();
	}
	// Optionally handle the case where the session destruction was not successful
}

void ULobbyWidget::ClientReturnToMenu()
{
	APlayerController* OwningPlayer = GetOwningPlayer();
	if (!OwningPlayer) return;

	FString Command = FString::Printf(TEXT("Open %s"), *PathToMenu);
	OwningPlayer->ClientTravel(Command, ETravelType::TRAVEL_Absolute);

	//MenuTearDown();
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
