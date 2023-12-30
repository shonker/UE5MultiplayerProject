// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcWall.h"

// Sets default values
AProcWall::AProcWall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AProcWall::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProcWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

