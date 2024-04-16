// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h" 
#include "Camera/CameraActor.h"
#include "TimerManager.h" 
#include "Blaster/HUD/LobbyWidget.h"



void ALobbyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    TArray<AActor*> FoundCameras;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACameraActor::StaticClass(), FoundCameras);

    if (FoundCameras.Num() > 0)
    {
        FindACamera();
    }
    else
    {
        FTimerHandle PosessCameraTimer;
        GetWorld()->GetTimerManager().SetTimer(PosessCameraTimer, this, &ALobbyPlayerController::FindACamera, 1.F, false);
    }

    // bShowMouseCursor = true;
    // SetInputMode(FInputModeUIOnly());

    // if (LobbyUIWidgetClass) // Ensure this class is set, typically via the editor or in the constructor
    // {
    //     UUserWidget* MyUI = CreateWidget<UUserWidget>(this, LobbyUIWidgetClass);
    //     if (MyUI)
    //     {
    //         MyUI->AddToViewport();
    //     }
    // }

}

void ALobbyPlayerController::FindACamera()
{
    TArray<AActor*> FoundCameras;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACameraActor::StaticClass(), FoundCameras);
    if (FoundCameras.Num() == 0) return;
    int32 Index = FMath::RandRange(0, FoundCameras.Num() - 1);
    AActor* SelectedCamera = FoundCameras[Index];
    if (SelectedCamera)
    {
        SetViewTarget(SelectedCamera);
    }
}


void ALobbyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (EndPlayReason == EEndPlayReason::LevelTransition || EndPlayReason == EEndPlayReason::EndPlayInEditor || EndPlayReason == EEndPlayReason::Quit)
    {
        if (LobbyWidget)
        {
            LobbyWidget->MenuTearDown();
        }
    }
    Super::EndPlay(EndPlayReason);
}

void ALobbyPlayerController::SetLobbyWidget(ULobbyWidget* Widget)
{
    LobbyWidget = Widget;
}