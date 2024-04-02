// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcRoom.h"

#include "Kismet/KismetMathLibrary.h"

// Struct Thas holds pair of selected transform and associated spawned actor
struct FTransformObj { FTransform SelectedTransform; AActor* SpawnedActor; };

void AProcRoom::ProcGen() {
    /*
     // Initialize arbitrary weights for each number of transforms
    TMap<int32, float> Weights;
    for (int32 i = 0; i <= MaxSpawnedObjects; ++i) {
        Weights.Add(i, FMath::Exp(-ObjectExpDecayRate * i));
    }

    // Normalize and accumulate the weights
    float SumWeights = 0.0f;
    for (auto Elem : Weights) {
        SumWeights += Elem.Value;
    }
    float AccumulatedWeight = 0.0f;
    TMap<int32, float> NormalizedWeights;
    for (auto Elem : Weights) {
        AccumulatedWeight += Elem.Value / SumWeights;
        NormalizedWeights.Add(Elem.Key, AccumulatedWeight);
    }

    // Generate a random number between 0 and 1
    float RandomNum = UKismetMathLibrary::RandomFloatInRangeFromStream(0.0f, 1.0f, RS);

    // Choose the number of transforms to select based on the random number
    int32 ChosenNumTransforms = -1;
    for (auto Elem : NormalizedWeights) {
        if (RandomNum <= Elem.Value) {
            ChosenNumTransforms = Elem.Key;
            break;
        }
    }

    TArray<FTransform> AllTransforms = FloorCenterOutputTransform;
    AllTransforms.Append(FloorWallSpawnableTransforms);
    AllTransforms.Append(MiddleWallSpawnableTransforms);

    // Container for selected transforms and spawned objects
    TArray<FTransformObj> SelectedTransforms;

    for (int32 i = 0; i < ChosenNumTransforms; i++) {
        // Select random transform
        int32 RandomTransformIndex = UKismetMathLibrary::RandomIntegerInRangeFromStream(0, AllTransforms.Num()-1, RS);
        FTransform SelectedTransform = AllTransforms[RandomTransformIndex];
        AllTransforms.RemoveAt(RandomTransformIndex);

        // Determine the associated spawnable object type by reverse finding the correct object types based on the transforms
        TArray<TSubclassOf<AActor>> AssociatedObjects;
        if (ContainsTransform(FloorCenterOutputTransform, SelectedTransform, 0.1f)) {
            AssociatedObjects = GetSpawnableObjectsForRoomType(RoomType, "cf");
        } else if (ContainsTransform(FloorWallSpawnableTransforms, SelectedTransform, 0.1f)) {
            AssociatedObjects = GetSpawnableObjectsForRoomType(RoomType, "fw");
        } else if (ContainsTransform(MiddleWallSpawnableTransforms, SelectedTransform, 0.1f)) {
            AssociatedObjects = GetSpawnableObjectsForRoomType(RoomType, "cw");
        }
        // Randomly select an actor class to spawn from the associated objects
        if (AssociatedObjects.Num() > 0) {
            int32 RandomActorIndex = UKismetMathLibrary::RandomIntegerInRangeFromStream(0, AssociatedObjects.Num() - 1, RS);
            TSubclassOf<AActor> SpawnableObject = AssociatedObjects[RandomActorIndex];
            FActorSpawnParameters SpawnParams;
            AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(SpawnableObject, SelectedTransform, SpawnParams);

            // Add to results array
            SelectedTransforms.Add({SelectedTransform, SpawnedActor});
        }
    }

    // SelectedTransforms Array now contains selected transform and associated spawned actor pairs
    */
}
/*
TArray<FPlacementTypeObject> AProcRoom::GetSpawnableObjectsForRoomType(ERoomType RoomType, const FName& PlacementTag)
{
    TArray<FPlacementTypeObject> FilteredObjects;
    for (const auto& RoomTypeSpawnable : RoomTypeSpawnables)
    {
        if (RoomTypeSpawnable.RoomType == RoomType) // Direct enum comparison
        {
            for (const auto& Obj : RoomTypeSpawnable.SpawnableObjects)
            {
                if (Obj.PlacementTag == PlacementTag)
                {
                    FilteredObjects.Add(Obj);
                }
            }
            break; // Found the matching room type, no need to continue
        }
    }
    return FilteredObjects;
}
*/
bool AProcRoom::AreTransformsEqual(const FTransform& Transform1, const FTransform& Transform2, float Epsilon)
{
    return Transform1.GetLocation().Equals(Transform2.GetLocation(), Epsilon)
           && Transform1.GetRotation().Equals(Transform2.GetRotation(), Epsilon)
           && Transform1.GetScale3D().Equals(Transform2.GetScale3D(), Epsilon);
}

bool AProcRoom::ContainsTransform(const TArray<FTransform>& TransformArray, const FTransform& TransformToCheck, float Epsilon)
{
    for (const auto& Transform : TransformArray)
    {
        if (AreTransformsEqual(Transform, TransformToCheck, Epsilon))
        {
            return true;
        }
    }

    return false;
}