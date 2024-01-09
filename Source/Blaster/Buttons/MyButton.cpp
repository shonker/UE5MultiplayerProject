// Fill out your copyright notice in the Description page of Project Settings.


#include "MyButton.h"

// Sets default values
AMyButton::AMyButton()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyButton::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyButton::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

