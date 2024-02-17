// Fill out your copyright notice in the Description page of Project Settings.


#include "CTowerPlatform.h"
#include "Math/UnrealMathUtility.h"

void ACTowerPlatform::ProcGen()
{
	int32 TowerCount = RS.FRandRange(6, 10);
	for (int32 i = 0; i < TowerCount; i++)
	{
		int32 OutputIndex = RS.RandRange(0, OutputTransforms.Num() - 1);
		FRotator Rotation = OutputTransforms[OutputIndex].GetRotation().Rotator();
		Rotation.Yaw = RS.RandRange(-180, 180);
		FVector Location = OutputTransforms[OutputIndex].GetLocation();

		int32 TowerIndex = RS.RandRange(0, TowerTypes.Num() - 1);

		ACurvedTower* SpawnedTower = Cast<ACurvedTower>(SpawnAt(TowerTypes[TowerIndex].ObjectBlueprint, Location, Rotation));

	}
}

void ACTowerPlatform::BeginPlay()
{
	Super::BeginPlay();
	
}