
// HomeBase.cpp

#include "HomeBase.h"
#include "Components/BoxComponent.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Net/UnrealNetwork.h"

AHomeBase::AHomeBase()
{
	bReplicates = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// Initialize the box component
	//this is weird. I think I made this so the game would start when you left home base but its not implemented correctly or intended for that OG purpose.
	//may repurpose for identifying when people enter home base to cure curses or kill enemies
	OverlapComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapComponent"));
	OverlapComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	OverlapComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	OverlapComponent->OnComponentBeginOverlap.AddDynamic(this, &AHomeBase::OnOverlapBegin);
}

void AHomeBase::BeginPlay()
{
	Super::BeginPlay();
	MamaActor->SetHomeBase(this);
	StartMatchCountdown();
}

void AHomeBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHomeBase, bLights);
	DOREPLIFETIME(AHomeBase, bRedLights);
}


void AHomeBase::OnRep_RedLights()
{
	RedLights();
}

void AHomeBase::OnRep_ActivateLights()
{
	ActivateLights();
}

void AHomeBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

void AHomeBase::StartMatchCountdown()
{
	// Access the game mode and start the countdown
	if (ABlasterGameMode* GM = Cast<ABlasterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->StartCountdown();
	}
}

void AHomeBase::BeginJudgement()
{
	bLights = true;
	ActivateLights();
	FTimerHandle MamaTimer;
	//takes 6 seconds for all the lights to come on
	GetWorldTimerManager().SetTimer(MamaTimer, this, &AHomeBase::ActivateMama, 6.0f, false);	
}

void AHomeBase::MamaStarved()
{
	bRedLights = true;
	RedLights();
}

void AHomeBase::ActivateMama()
{
	if (MamaActor)
	{
		MamaActor->BeginActivatingState();
	}
}
