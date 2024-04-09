// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StartupPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AStartupPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	void FindACamera();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> StartupUIWidgetClass;

protected:
	// Override the EndPlay function
	//virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	//class ULobbyWidget* LobbyWidget = nullptr;

public:
	//void SetStartupWidget(ULobbyWidget* Widget);
};
