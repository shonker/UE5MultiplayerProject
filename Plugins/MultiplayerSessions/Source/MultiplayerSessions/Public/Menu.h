// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Menu.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FBlueprintSessionInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FString SessionName;

	UPROPERTY(BlueprintReadOnly)
	FString OwningUserName;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentPlayers;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayers;

	// Constructor
	FBlueprintSessionInfo() : CurrentPlayers(0), MaxPlayers(0) {}
};

UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("/Game/Maps/Lobby")));

	UFUNCTION(BlueprintImplementableEvent, Category = "Menu")
	void CreateSessionList(const TArray<FBlueprintSessionInfo>& SessionResults);

	UFUNCTION(BlueprintCallable)
	void JoinSelectedSession(const FString& SessionId);

	UPROPERTY(BlueprintReadWrite)
	FString SelectedSessionId; 

protected:

	virtual bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

	//
	// Callbacks for the custom delegates on the MultiplayerSessionsSubsystem
	//
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:

	TArray<FOnlineSessionSearchResult> SearchResults;

	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* FindSessionsButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinSelectedButton;

	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void FindSessionsButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	void MenuTearDown();

	// The subsystem designed to handle all online session functionality
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	int32 NumPublicConnections{4};
	FString MatchType{TEXT("FreeForAll")};
	FString PathToLobby{TEXT("")};

public:
	UFUNCTION(BlueprintCallable, Category = "Menu")
	FBlueprintSessionInfo GetSessionInfoAt(int32 Index);
};
