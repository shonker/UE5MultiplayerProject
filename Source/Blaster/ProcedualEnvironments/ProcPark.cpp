#include "ProcPark.h"
#include "ProcParkPart.h"

void AProcPark::BeginPlay()
{
    if (HasAuthority())
    {      
        FVector Location;
        Location = FVector(857, 274, 1435);
        InitializePark(Location, FRotator(0, 0, FMath::RandRange(0, 1) * 180));
        Location = FVector(2006,1746,1436);
        InitializePark(Location, FRotator(0,0,-130));

        TArray<AActor*> OverlappingActors;
        GetOverlappingActors(OverlappingActors);
        for (AActor* Actor : OverlappingActors)
        {
                Actor->Destroy();
        }
    }
   
}

void AProcPark::InitializePark(FVector Location,FRotator Rotation)
{
    int32 Lifetime = FMath::RandRange(5, 10);

    FVector StartLocation = Location + GetActorLocation();
    FRotator StartRotation = Rotation;

    FTransform StartTransform(StartRotation, StartLocation);

    BranchCount = FMath::RandRange(3, MaxBranchCount);
    SpawnNextObject(StartTransform, Lifetime, 0);
}

void AProcPark::SpawnNextObject(const FTransform& ParentTransform, int32 CurrentLifetime, int32 CurrentBranchCount)
{
    if (CurrentLifetime <= 0 || ObjectTypes.Num() == 0)
        return;

    FObjectTypeInfo SelectedType = GetRandomObjectTypeExcluding(LastSpawnedParkPart);
    if (!SelectedType.ObjectBlueprint)
        return;

    AProcParkPart* SpawnedPart = Cast<AProcParkPart>(GetWorld()->SpawnActor<AProcParkPart>(SelectedType.ObjectBlueprint, ParentTransform.GetLocation(), ParentTransform.GetRotation().Rotator()));
    if (!SpawnedPart || SpawnedPart->OutputTransforms.Num() == 0)
        return;

    LastSpawnedParkPart = SelectedType.ObjectBlueprint;

    ProcessSpawn(SpawnedPart, ParentTransform, CurrentLifetime, CurrentBranchCount, false, SelectedType);

    if (SpawnedPart->OutputTransforms.Num() > 1 && BranchCount > 0 && FMath::RandBool())
    {
        ProcessSpawn(SpawnedPart, ParentTransform, CurrentLifetime, CurrentBranchCount + 1, true, SelectedType);
    }
}

void AProcPark::ProcessSpawn(AProcParkPart* SpawnedPart, const FTransform& ParentTransform, int32 CurrentLifetime, int32 CurrentBranchCount, bool IsBranch, FObjectTypeInfo SelectedType)
{
    int32 OutputIndex = FMath::RandRange(0, SpawnedPart->OutputTransforms.Num() - 1);
    if (IsBranch)
    {
        int32 OriginalIndex = OutputIndex;
        do {
            OutputIndex = FMath::RandRange(0, SpawnedPart->OutputTransforms.Num() - 1);
        } while (OutputIndex == OriginalIndex);
    }

    FTransform OutputTransform = SpawnedPart->OutputTransforms[OutputIndex];
    FTransform WorldTransform = OutputTransform * ParentTransform;

    float RandomYawOffset = FMath::RandRange(-1,1) * 90.0f; // Yaw offset
    FRotator AdjustedRotation = WorldTransform.GetRotation().Rotator();
    AdjustedRotation.Yaw += RandomYawOffset;
    AdjustedRotation.Roll += FMath::RandRange(0, 1) * 180; // Additional flip

    WorldTransform.SetRotation(AdjustedRotation.Quaternion());

    if (CurrentLifetime > 1)
    {
        FTransform NextParentTransform = WorldTransform;
        SpawnNextObject(NextParentTransform, CurrentLifetime - 1, CurrentBranchCount);
    }
}


FVector AProcPark::CalculateOutputLocation(const AActor* SpawnedObject, const FVector& OutputLocation, const FRotator& Rotation)
{
    if (!SpawnedObject)
        return FVector::ZeroVector;

    return Rotation.RotateVector(OutputLocation) + SpawnedObject->GetActorLocation();
}

FObjectTypeInfo AProcPark::GetRandomObjectTypeExcluding(const TSubclassOf<AProcParkPart>& ExcludeType)
{
    TArray<FObjectTypeInfo> PossibleTypes;

    for (const FObjectTypeInfo& TypeInfo : ObjectTypes)
    {
        if (TypeInfo.ObjectBlueprint != ExcludeType)
        {
            PossibleTypes.Add(TypeInfo);
        }
    }

    if (PossibleTypes.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, PossibleTypes.Num() - 1);
        return PossibleTypes[RandomIndex];
    }
    return FObjectTypeInfo();
}