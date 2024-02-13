// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void LobbySetup(FString MenuPath = FString(TEXT("/Game/ThirdPersonCPP/Maps/StartupMap")));
	void MenuTearDown();

protected:
	virtual bool Initialize() override;

private:
	UPROPERTY()
	class ALobbyGameMode* LobbyGameMode;

	UPROPERTY(meta = (BindWidget))
	class UButton* LeaveButton;

	UPROPERTY(meta = (BindWidget))
	UButton* StartButton;

	UFUNCTION()
	void StartButtonClicked();

	UFUNCTION()
	void LeaveButtonClicked();

	FString PathToMenu{ TEXT("") };

};
