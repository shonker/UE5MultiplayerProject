// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	//AddToViewport();
	//SetVisibility(ESlateVisibility::Visible);
	//bIsFocusable = true;

	// UWorld* World = GetWorld();
	// if (World)
	// {
	// 	APlayerController* PlayerController = World->GetFirstPlayerController();
	// 	if (PlayerController)
	// 	{
	// 		FInputModeUIOnly InputModeData;
	// 		InputModeData.SetWidgetToFocus(TakeWidget());
	// 		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	// 		PlayerController->SetInputMode(InputModeData);
	// 		PlayerController->SetShowMouseCursor(true);
	// 	}
	// }

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}
	

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if (FindSessionsButton)
	{
		FindSessionsButton->OnClicked.AddDynamic(this, &ThisClass::FindSessionsButtonClicked);
	}
	if (JoinSelectedButton)
	{
		JoinSelectedButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}

	return true;
}

void UMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(PathToLobby);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Failed to create session!"))
			);
		}
		HostButton->SetIsEnabled(true);
	}
}

void UMenu::FindSessionsButtonClicked()
{
	FindSessionsButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSessions(10000);
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Found %d sessions."), SessionResults.Num()));
	}

	// Clear previous search results
	SearchResults.Empty();

	if (bWasSuccessful && SessionResults.Num() > 0)
	{
		// Populate the SearchResults array for future reference, such as joining a session
		SearchResults = SessionResults;

		TArray<FBlueprintSessionInfo> BlueprintSessions;
		for (const FOnlineSessionSearchResult& SearchResult : SessionResults)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					10.f,
					FColor::Red,
					FString::Printf(
						TEXT("Session ID: %s, Owner: %s"),
						*SearchResult.GetSessionIdStr(),
						*SearchResult.Session.OwningUserName
					)
				);
			}

			FBlueprintSessionInfo Info;
			Info.SessionName = SearchResult.GetSessionIdStr();
			Info.OwningUserName = SearchResult.Session.OwningUserName;
			Info.CurrentPlayers = SearchResult.Session.SessionSettings.NumPublicConnections - SearchResult.Session.NumOpenPublicConnections;
			Info.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;

			BlueprintSessions.Add(Info);
		}
		CreateSessionList(BlueprintSessions);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No sessions found or search failed."));
	}
}

void UMenu::JoinButtonClicked()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Red,
			FString::Printf(TEXT("JOIN BUTTON CLICKED"))
		);
	}
	UE_LOG(LogTemp, Error, TEXT("JOIN BUTTON CLICKED"));

	JoinSelectedSession(SelectedSessionId);//this is the index, not the ID you nincompoop
}

void UMenu::JoinSelectedSession(const FString& SessionId)
{
	if (MultiplayerSessionsSubsystem)
	{
		// Find the session by ID
		for (const FOnlineSessionSearchResult& SearchResult : SearchResults)
		{
			if (SearchResult.GetSessionIdStr() == SessionId)
			{
				MultiplayerSessionsSubsystem->JoinSession(SearchResult);
				return;
			}
		}

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString::Printf(TEXT("Failed to find selected session ID: %s"), *SessionId)
			);
		}
	}
}


void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::HostButtonClicked()
{
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("no mss"));
	}
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}

FBlueprintSessionInfo UMenu::GetSessionInfoAt(int32 Index)
{
	FBlueprintSessionInfo Info;

	if (SearchResults.IsValidIndex(Index))
	{
		const auto& SearchResult = SearchResults[Index];
		Info.SessionName = SearchResult.GetSessionIdStr();
		Info.OwningUserName = SearchResult.Session.OwningUserName;
		Info.CurrentPlayers = SearchResult.Session.SessionSettings.NumPublicConnections - SearchResult.Session.NumOpenPublicConnections;
		Info.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
	}
	return Info;
}
