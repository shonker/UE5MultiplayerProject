#include "ProcPark.h"

void AProcPark::BeginPlay()
{
    if (HasAuthority())
    {
        // Randomly choose a lifetime between 5 and 10
        int32 Lifetime = FMath::RandRange(5, 10);

        FVector StartLocation = FVector(857, 274, 1435) + GetActorLocation();
        FRotator StartRotation = FRotator(0, 0, FMath::RandRange(0, 1) * 180);

        // Combine StartLocation and StartRotation into a FTransform
        FTransform StartTransform(StartRotation, StartLocation);

        BranchCount = FMath::RandRange(2, MaxBranchCount);
        SpawnNextObject(StartTransform, Lifetime, 0);
    }
}
void AProcPark::SpawnNextObject(const FTransform& ParentTransform, int32 CurrentLifetime, int32 CurrentBranchCount)
{
    if (CurrentLifetime <= 0 || ObjectTypes.Num() == 0)
        return;

    FObjectTypeInfo SelectedType = GetRandomObjectTypeExcluding(LastSpawnedParkPart);
    if (!SelectedType.ObjectBlueprint)
        return;

    LastSpawnedParkPart = SelectedType.ObjectBlueprint;

    AProcParkPart* SpawnedPart = Cast<AProcParkPart>(SelectedType.ObjectBlueprint);
    if (SpawnedPart && SpawnedPart->OutputTransforms.Num() > 0)
    {
        int32 OutputIndex = FMath::RandRange(0, SpawnedPart->OutputTransforms.Num() - 1);
        FTransform OutputTransform = SpawnedPart->OutputTransforms[OutputIndex];
        FTransform WorldTransform = OutputTransform * ParentTransform;
        AdjustRotationAndSpawn(WorldTransform, SelectedType, CurrentLifetime, CurrentBranchCount);

        // Check for branching possibility
        if (SpawnedPart->OutputTransforms.Num() > 1 && FMath::RandBool())
        {
            // Select a different output transform for branching
            int32 BranchOutputIndex;
            do {
                BranchOutputIndex = FMath::RandRange(0, SpawnedPart->OutputTransforms.Num() - 1);
            } while (BranchOutputIndex == OutputIndex); // Ensure different index

            FTransform BranchOutputTransform = SpawnedPart->OutputTransforms[BranchOutputIndex];
            FTransform BranchWorldTransform = BranchOutputTransform * ParentTransform;
            AdjustRotationAndSpawn(BranchWorldTransform, SelectedType, CurrentLifetime, CurrentBranchCount + 1);
        }
    }
}

void AProcPark::AdjustRotationAndSpawn(const FTransform& WorldTransform, const FObjectTypeInfo& SelectedType, int32 CurrentLifetime, int32 CurrentBranchCount)
{
    // Adjust rotation with random yaw offset (-90, 0, 90)
    float RandomYawOffset = FMath::RandRange(-1, 1) * 90.0f;
    FRotator AdjustedRotation = WorldTransform.GetRotation().Rotator();
    AdjustedRotation.Yaw += RandomYawOffset;

    // Spawn the next object with the adjusted transform
    AActor* SpawnedObject = GetWorld()->SpawnActor<AActor>(SelectedType.ObjectBlueprint, WorldTransform.GetLocation(), AdjustedRotation);
    if (SpawnedObject)
    {

        // Update the transform for the next spawn
        FTransform NextParentTransform = SpawnedObject->GetActorTransform();

        // Continue spawning if lifetime and branch count permit
        if (CurrentLifetime > 1 && CurrentBranchCount < MaxBranchCount)
        {
            SpawnNextObject(NextParentTransform, CurrentLifetime - 1.f, CurrentBranchCount);
        }
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