// Include statements for Bedroom.h, LobbyCharacter.h, and any other necessary UE headers
#include "Bedroom.h"
#include "LobbyCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h" 

ABedroom::ABedroom()
{

}

void ABedroom::BeginPlay()
{
    Super::BeginPlay();
}

void ABedroom::SpawnLobbyCharacter(APlayerController* NewPlayer)
{
    if (!GetWorld() || !LobbyCharacterBP) return; // Safety check

    FVector SpawnLocation = FVector(0.0f, 0.0f, 0.0f); 
    FRotator SpawnRotation = FRotator::ZeroRotator; 
    FActorSpawnParameters SpawnParams;

    ALobbyCharacter* SpawnedCharacter = GetWorld()->SpawnActor<ALobbyCharacter>(LobbyCharacterBP, SpawnLocation, SpawnRotation, SpawnParams);

    if (SpawnedCharacter)
    {
        SpawnedCharacter->SetOwningPlayer(NewPlayer);
        SpawnedCharacter->SelectCharacterMeshAndLocation(LobbyCharacters);
        LobbyCharacters.Add(SpawnedCharacter);
    }
}

void ABedroom::Shake()
{
    // Find all Camera Actors in the level
    TArray<AActor*> Cameras;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACameraActor::StaticClass(), Cameras);

    for (AActor* Camera : Cameras)
    {
        ACameraActor* CameraActor = Cast<ACameraActor>(Camera);
        if (CameraActor)
        {
            FVector OriginalLocation = CameraActor->GetActorLocation();

            // Define a lambda function to shake the camera
            auto ShakeCamera = [CameraActor, OriginalLocation]()
            {
                FVector NewLocation = OriginalLocation + FMath::VRand() * 30.f; // Shake magnitude
                CameraActor->SetActorLocation(NewLocation);
            };

            // Shake the camera immediately
            ShakeCamera();

            // Set a timer to reset the camera's location after 0.2 seconds
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [CameraActor, OriginalLocation]()
                {
                    CameraActor->SetActorLocation(OriginalLocation);
                }, 0.2f, false);
        }
    }
}

