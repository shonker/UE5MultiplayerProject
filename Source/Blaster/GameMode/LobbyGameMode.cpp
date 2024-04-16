#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "Blaster/Bedroom/Bedroom.h"
#include "Blaster/Bedroom/LobbyCharacter.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h" 
#include "Kismet/GameplayStatics.h"
#include "Blaster/HUD/LobbyWidget.h"


/*
    NOTE: game mode only exists on server
*/

void ALobbyGameMode::BeginPlay()
{
    FoundBedroom = FindBedroomActor();
    SpawnHostPlayerCharacter();
}

void ALobbyGameMode::SpawnHostPlayerCharacter()
{
    UWorld* World = GetWorld();
    if (!World) return;
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (PlayerController
        && PlayerController->HasAuthority()
        && FoundBedroom)
    {
        FoundBedroom->SpawnLobbyCharacter(PlayerController);
    }
}

ABedroom* ALobbyGameMode::FindBedroomActor()
{
    for (TActorIterator<ABedroom> It(GetWorld()); It; ++It)
    {
        FoundBedroom = *It;
        break;
    }
    return FoundBedroom;
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer); 

    if (FoundBedroom != nullptr && HasAuthority())
    {
        FoundBedroom->SpawnLobbyCharacter(NewPlayer);
    }
}

void ALobbyGameMode::StartGame()
{   
    if (FoundBedroom != nullptr)
    {
        for (ALobbyCharacter* Character : FoundBedroom->LobbyCharacters)
        {
            if (Character)
            {
                Character->bAsleep = true;
            }
        }
    }

    // Set a timer for level travel
    FTimerHandle LevelTravelTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(LevelTravelTimerHandle, this, &ALobbyGameMode::TravelToNextLevel, 5.0f, false);
}

void ALobbyGameMode::TravelToNextLevel()
{
    UWorld* World = GetWorld();
    if (World)
    {
        bUseSeamlessTravel = true;
        World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
    }
}

void ALobbyGameMode::Logout(AController* Exiting)
{
    APlayerController* ExitingPlayer = Cast<APlayerController>(Exiting);
    
    if (FoundBedroom != nullptr)
    {
        for (int32 i = FoundBedroom->LobbyCharacters.Num() - 1; i >= 0; --i)
        {
            if (FoundBedroom->LobbyCharacters[i]->GetOwningPlayer() == ExitingPlayer)
            {
                FoundBedroom->LobbyCharacters[i]->Destroy();
                FoundBedroom->LobbyCharacters.RemoveAt(i);
                break;
            }
        }
    }

    Super::Logout(Exiting);
}