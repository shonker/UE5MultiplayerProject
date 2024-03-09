
// HomeBase.cpp

#include "HomeBase.h"
#include "Components/BoxComponent.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Net/UnrealNetwork.h"

AHomeBase::AHomeBase()
{
	SetReplicates(true);
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// Initialize the box component
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
	StartMatchCountdown();
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
