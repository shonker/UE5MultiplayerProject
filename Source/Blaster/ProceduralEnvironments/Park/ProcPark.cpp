#include "ProcPark.h"
#include "ProcParkPart.h"
#include "EngineUtils.h"  
#include "Blaster/Monsters/TubeSlide/TubeSlide.h"
#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"

void AProcPark::BeginPlay()
{
    Super::BeginPlay();
}

AProcPark::AProcPark()
{
    Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    SetRootComponent(Root);

    StartingPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("StartingPoint"));
    StartingPoint->SetupAttachment(RootComponent);
    StartingPoint->SetRelativeLocation(FVector(0.f, 0.f, 700.f)); 
}

void AProcPark::ProcGen()
{
    InitializePark();
}


void AProcPark::InitializePark()
{
    if (!StartingPoint) return;
    FTransform StartTransform = StartingPoint->GetComponentTransform();
    Lifetime = RS.RandRange(10,20);
    SpawnNextObject(StartTransform);
}
void AProcPark::SpawnNextObject(const FTransform& CurrentTransform)
{
    if (Lifetime-- < 0) return; // Stop recursion if the lifetime limit has been reached
    if (ObjectTypes.Num() == 0) return; // Do nothing if there are no object types to spawn

    // Select a random type from the available object types
    FObjectTypeInfo SelectedType = ObjectTypes[RS.RandRange(0, ObjectTypes.Num() - 1)];
    FRotator NewRotation = CurrentTransform.GetRotation().Rotator();
    NewRotation.Roll += RS.RandRange(0, 1) * 180.f; // Apply 0 or 180 degrees roll
    NewRotation.Yaw += RS.RandRange(-1, 1) * 90.f; // Apply -90, 0, or 90 degrees yaw
    NewRotation.Normalize(); // Normalize the rotation to ensure valid rotation values

    // Spawn the park part at the current transform's location with the new rotation
    AProcParkPart* SpawnedPart = GetWorld()->SpawnActor<AProcParkPart>(
        SelectedType.ObjectBlueprint, 
        CurrentTransform.GetLocation(), 
        NewRotation
    );

    if (!SpawnedPart || SpawnedPart->OutputTransforms.Num() == 0)
        return; // Exit if spawn failed or no output transforms are available

    // Select a random output transform to determine where to spawn the next part
    int32 SelectedIndex = RS.RandRange(0, SpawnedPart->OutputTransforms.Num() - 1);
    FTransform ChosenTransform = SpawnedPart->OutputTransforms[SelectedIndex] * SpawnedPart->GetActorTransform();

    // Add non-selected output transforms to the list of available transforms for future use
    for (int i = 0; i < SpawnedPart->OutputTransforms.Num(); ++i)
    {
        if (i != SelectedIndex)
        {
            AvailableTransforms.Add(SpawnedPart->OutputTransforms[i] * SpawnedPart->GetActorTransform());
        }
    }

    // Check if the chosen transform is within the allowed distance
    if (!IsWithinDistance(ChosenTransform.GetLocation()))
    {
       // UE_LOG(LogTemp, Warning, TEXT("Selected position isn't within the distance"));
        if (AvailableTransforms.Num() > 0)
        {
            // If out of bounds, select a new transform from the available transforms
            int32 RandomIndex = RS.RandRange(0, AvailableTransforms.Num() - 1);
            ChosenTransform = AvailableTransforms[RandomIndex];
            AvailableTransforms.RemoveAt(RandomIndex);
        }
        else
        {
            //UE_LOG(LogTemp, Error, TEXT("No available transforms left"));
            return; // Exit if no valid transforms are available
        }
    }

    // Continue spawning the next object recursively
    SpawnNextObject(ChosenTransform);
}

bool AProcPark::IsWithinDistance(const FVector& Position) const
{
    if (!StartingPoint)
    {
       // UE_LOG(LogTemp, Warning, TEXT("StartingPoint is null"));
        return false; 
    }
    FVector StartVector = StartingPoint->GetComponentLocation();

    FVector2D XYOrigin(StartVector.X, StartVector.Y);
    FVector2D XYPosition(Position.X, Position.Y);

    float DistanceXY = FVector2D::Distance(XYOrigin, XYPosition);

    bool bIsWithinDistance = DistanceXY <= 1500.f;
    if (!bIsWithinDistance) 
    {
        //UE_LOG(LogTemp, Error, TEXT("Position too far on xy plane"));
    }

    bool bIsAboveGround = Position.Z > -400.f;

    return bIsWithinDistance && bIsAboveGround;
}
