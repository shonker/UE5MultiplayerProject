#include "ProcPark.h"
#include "ProcParkPart.h"
#include "EngineUtils.h"  
#include "Blaster/Monsters/TubeSlide/TubeSlide.h"

void AProcPark::BeginPlay()
{
    Super::BeginPlay();
}

void AProcPark::ProcGen()
{
    FVector Location;
    Location = FVector(857, 274, 1435);
    InitializePark(Location, FRotator(0, 0, RS.RandRange(0, 1) * 180));
    Location = FVector(2006, 1746, 1436);
    InitializePark(Location, FRotator(0, 0, 0));

    // Define the bounds
    FVector MinBounds = FVector(-1000, -1000, 0) + GetActorLocation();
    FVector MaxBounds = FVector(3000, 3000, 3000) + GetActorLocation();

    // Iterate over all actors of type AProcParkPart
    for (TActorIterator<AProcParkPart> It(GetWorld()); It; ++It)
    {
        AProcParkPart* ParkPart = *It;
        if (ParkPart)
        {
            // Check if the actor's position is outside the bounds
            if (!IsWithinBounds(ParkPart->GetActorLocation(), MinBounds, MaxBounds))
            {
                // UE_LOG(LogTemp, Error, TEXT("DESTROY@!@"));
                // Destroy the actor if it's outside the bounds
                ParkPart->Destroy();
            }
        }
    }
}

bool AProcPark::IsWithinBounds(const FVector & Position, const FVector & MinBounds, const FVector & MaxBounds)
{
    return Position.X >= MinBounds.X && Position.X <= MaxBounds.X &&
        Position.Y >= MinBounds.Y && Position.Y <= MaxBounds.Y &&
        Position.Z >= MinBounds.Z && Position.Z <= MaxBounds.Z;
}

void AProcPark::InitializePark(FVector Location,FRotator Rotation)
{
    int32 Lifetime = RS.RandRange(5, 10);

    FVector StartLocation = Location + GetActorLocation();
    FRotator StartRotation = Rotation;

    FTransform StartTransform(StartRotation, StartLocation);

    BranchCount = RS.RandRange(0, MaxBranchCount);
    SpawnNextObject(StartTransform, Lifetime, 0);
}

void AProcPark::SpawnNextObject(const FTransform& ParentTransform, int32 CurrentLifetime, int32 CurrentBranchCount)
{
    if (CurrentLifetime <= 0 || ObjectTypes.Num() == 0)
        return;

    FObjectTypeInfo SelectedType = GetRandomObjectTypeExcluding(LastSpawnedParkPart);
    if (!SelectedType.ObjectBlueprint)
        return;

    if (CurrentLifetime == 1 && RS.RandRange(0, 1) == 1)
    {
        AActor* SpawnedHouse = GetWorld()->SpawnActor<AActor>(TubeSlideBlueprint, ParentTransform.GetLocation(), ParentTransform.GetRotation().Rotator());
        return;
    }
    FVector Location = ParentTransform.GetLocation();
    FRotator Rotation = ParentTransform.GetRotation().Rotator();
    AProcParkPart* SpawnedPart = Cast<AProcParkPart>(SpawnAt(SelectedType.ObjectBlueprint, Location, Rotation));
    if (!SpawnedPart || SpawnedPart->OutputTransforms.Num() == 0)
        return;

    LastSpawnedParkPart = SelectedType.ObjectBlueprint;

    ProcessSpawn(SpawnedPart, ParentTransform, CurrentLifetime, CurrentBranchCount, false, SelectedType);

    if (SpawnedPart->OutputTransforms.Num() > 1 && BranchCount > 0 && RS.RandRange(0, 1) == 1)
    {
        ProcessSpawn(SpawnedPart, ParentTransform, CurrentLifetime, CurrentBranchCount + 1, true, SelectedType);
    }
}

void AProcPark::ProcessSpawn(AProcParkPart* SpawnedPart, const FTransform& ParentTransform, int32 CurrentLifetime, int32 CurrentBranchCount, bool IsBranch, FObjectTypeInfo SelectedType)
{
    int32 OutputIndex = RS.RandRange(0, SpawnedPart->OutputTransforms.Num() - 1);
    if (IsBranch)
    {
        int32 OriginalIndex = OutputIndex;
        do {
            OutputIndex = RS.RandRange(0, SpawnedPart->OutputTransforms.Num() - 1);
        } while (OutputIndex == OriginalIndex);
    }

    FTransform OutputTransform = SpawnedPart->OutputTransforms[OutputIndex];
    FTransform WorldTransform = OutputTransform * ParentTransform;

    float RandomYawOffset = RS.RandRange(-1,1) * 90.0f; // Yaw offset
    FRotator AdjustedRotation = WorldTransform.GetRotation().Rotator();
    AdjustedRotation.Yaw += RandomYawOffset;
    AdjustedRotation.Roll += RS.RandRange(0, 1) * 180; // Additional flip

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
        int32 RandomIndex = RS.RandRange(0, PossibleTypes.Num() - 1);
        return PossibleTypes[RandomIndex];
    }
    return FObjectTypeInfo();
}