// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcFurniture.h"


void AProcFurniture::ProcGen()
{
	Super::ProcGen();
	GenerateFurniture();
}

void AProcFurniture::GenerateFurniture()
{
	const int32 NumTransforms = SpawnableObjectTransforms.Num();
	TArray<int32> AvailableIndices;
	for (int32 i = 0; i < NumTransforms; ++i) AvailableIndices.Add(i);
	const int32 ObjectsToSpawn = CalculateObjectsToSpawn(NumTransforms);
	//UE_LOG(LogTemp, Warning, TEXT("obj to spawn:%i"), ObjectsToSpawn);

	for (int32 i = 0; i < ObjectsToSpawn && AvailableIndices.Num() > 0; ++i)
	{
		if (SpawnableObjects.Num() > 0)
		{
			const int32 ObjectIndex = RS.RandRange(0, SpawnableObjects.Num() - 1);
			const int32 TransformIndex = RS.RandRange(0, AvailableIndices.Num() - 1);

			FTransform SpawnTransform = SpawnableObjectTransforms[AvailableIndices[TransformIndex]];
			FVector Location = GetActorLocation() + GetActorRotation().RotateVector(SpawnTransform.GetLocation());
			FRotator Rotation = GetActorRotation() + SpawnTransform.GetRotation().Rotator();
			if (bRandomRotateYawAndRollOfGeneratedFurniture)
			{
				Rotation.Yaw += 90.f * RS.RandRange(-1,1);
				Rotation.Roll += 90.f * RS.RandRange(-1,1);
			}
			Rotation.Yaw -= 90.0f;

			AAProcActor* ProcActor = SpawnAt(SpawnableObjects[ObjectIndex].ObjectClass, Location, Rotation);
			ProcActor->ProcGen();
			AvailableIndices.RemoveAt(TransformIndex);
		}
	}
}
int32 AProcFurniture::CalculateObjectsToSpawn(int32 NumTransforms) const
{
	if (NumTransforms <= 0)
	{
		return 0;
	}
	int32 RandomInt = RS.RandRange(0.0f, NumTransforms);
	int32 NewRandomInt = RS.RandRange(0,RandomInt);

	UE_LOG(LogTemp, Warning, TEXT("%i"), NewRandomInt);
	return NewRandomInt;
}