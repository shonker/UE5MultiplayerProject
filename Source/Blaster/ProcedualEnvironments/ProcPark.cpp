#include "ProcPark.h"

AProcPark::AProcPark()
{

}

void AProcPark::BeginPlay()
{
    // Randomly choose a lifetime between 5 and 10
    int32 Lifetime = FMath::RandRange(5, 10);
    FVector StartLocation(200, 500, 1100);
    FRotator StartRotation = FRotator(0, FMath::RandRange(0, 3) * 90, FMath::RandRange(0, 3) * 90);

    SpawnNextObject(StartLocation, StartRotation, Lifetime, 0);
}

void AProcPark::SpawnNextObject(const FVector& Location, const FRotator& Rotation, int32 CurrentLifetime, int32 CurrentBranchCount)
{
    if (CurrentLifetime <= 0 || ObjectTypes.Num() == 0)
        return;

    FObjectTypeInfo SelectedType = GetRandomObjectTypeExcluding(LastSpawnedParkPart);
   
    if (!SelectedType.ObjectBlueprint)
        return;

    // Spawn the object first, then get its output locations
    AActor* SpawnedObject = GetWorld()->SpawnActor<AActor>(SelectedType.ObjectBlueprint, Location, Rotation);
    if (!SpawnedObject)
        return;

    LastSpawnedParkPart = SelectedType.ObjectBlueprint; // Update last spawned part type

    AProcParkPart* SpawnedPart = Cast<AProcParkPart>(SpawnedObject);
    if (SpawnedPart && SpawnedPart->OutputLocations.Num() > 0)
    {
        // Randomly select one of the output locations
        int32 OutputIndex = FMath::RandRange(0, SpawnedPart->OutputLocations.Num() - 1);
        FVector OutputLocation = SpawnedPart->OutputLocations[OutputIndex];

        FVector NextLocation = CalculateOutputLocation(SpawnedObject, OutputLocation, Rotation);
        FRotator NextRotation = FRotator(0, FMath::RandRange(0, 3) * 90, FMath::RandRange(0, 3) * 90);

        // Debug visuals
        DrawDebugSphere(GetWorld(), NextLocation, 50.0f, 12, FColor::Red, true, -1.0f, 0, 2.0f);
        DrawDebugLine(GetWorld(), Location, NextLocation, FColor::Blue, true, -1.0f, 0, 5.0f);
        // Decide whether to branch
        bool ShouldBranch = false; /* Branching Logic */;
        if (ShouldBranch && CurrentBranchCount < MaxBranchCount)
        {
            // Spawn another branch
            SpawnNextObject(NextLocation, NextRotation, CurrentLifetime - 1, CurrentBranchCount + 1);
        }

        // Continue the current branch
        SpawnNextObject(NextLocation, NextRotation, CurrentLifetime - 1, CurrentBranchCount);
    }
}

// Update CalculateOutputLocation to take an additional FVector parameter for OutputLocation
FVector AProcPark::CalculateOutputLocation(const AActor* SpawnedObject, const FVector& OutputLocation, const FRotator& Rotation)
{
    if (!SpawnedObject)
        return FVector::ZeroVector;

    // Convert OutputLocation from local to world space
    return Rotation.RotateVector(OutputLocation) + SpawnedObject->GetActorLocation();
}

FObjectTypeInfo AProcPark::GetRandomObjectTypeExcluding(const TSubclassOf<AProcParkPart>& ExcludeType)
{
    TArray<FObjectTypeInfo> PossibleTypes;

    // Filter out the type to exclude
    for (const FObjectTypeInfo& TypeInfo : ObjectTypes)
    {
        if (TypeInfo.ObjectBlueprint != ExcludeType)
        {
            PossibleTypes.Add(TypeInfo);
        }
    }

    if (PossibleTypes.Num() > 0)
    {
        // Randomly select from the remaining types
        int32 RandomIndex = FMath::RandRange(0, PossibleTypes.Num() - 1);
        return PossibleTypes[RandomIndex];
    }

    // Return an empty struct if no valid type is found
    return FObjectTypeInfo();
}