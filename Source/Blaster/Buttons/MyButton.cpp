#include "MyButton.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"

AMyButton::AMyButton()
{
    PrimaryActorTick.bCanEverTick = true;

    AreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaBox"));
    SetRootComponent(AreaBox);
    AreaBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    InteractWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractWidget"));
    InteractWidget->SetupAttachment(RootComponent);
    bReplicates = true;
}

void AMyButton::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        AreaBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        AreaBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
        AreaBox->OnComponentBeginOverlap.AddDynamic(this, &AMyButton::OnSphereOverlap);
        AreaBox->OnComponentEndOverlap.AddDynamic(this, &AMyButton::OnSphereEndOverlap);
    }
    AreaBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    AreaBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

    if (InteractWidget)
    {
        InteractWidget->SetVisibility(false);
    }

    OwningActor = GetOwner();
}

void AMyButton::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (IsActivelyPressed) WhileHeld();
}

void AMyButton::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherComp->GetName() == FString("InteractSphere"))
    {
        ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
        if (BlasterCharacter)
        {
            UE_LOG(LogTemp, Log, TEXT("overlap detected in button on server"));
            BlasterCharacter->SetOverlappingButton(this);
        }
    }
}

void AMyButton::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherComp->GetName() == FString("InteractSphere"))
    {
        ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
        if (BlasterCharacter)
        {
            BlasterCharacter->SetOverlappingButton(nullptr);
        }
    }

    if (IsActivelyPressed)
    {
        IsActivelyPressed = false;
        OnDragOff();
    }
}

void AMyButton::MulticastOnInitPress_Implementation()
{
    IsActivelyPressed = true;
}

//technically this only needs to be populated with multicast oninit press as the server is the only one receiving the input from the clients. 
//no client should be calling these functions, however, i shall leave this here as i spent the time writing it out and may want it one day
void AMyButton::ServerOnInitPress_Implementation()
{
    OnButtonPressed.Broadcast();
   //MulticastOnInitPress();
}

void AMyButton::OnInitPress()
{
   /* if (HasAuthority())
    {*/
    OnButtonPressed.Broadcast();
    //   // MulticastOnInitPress();
    //}
    //else
    //{
    //    ServerOnInitPress();
    //}
}

void AMyButton::MulticastWhileHeld_Implementation()
{

}

void AMyButton::ServerWhileHeld_Implementation()
{
}

void AMyButton::WhileHeld()
{
    //not really necessarry
   // OnButtonHeld.Broadcast();
}

void AMyButton::MulticastOnRelease_Implementation()
{
    OnButtonReleased.Broadcast();
    IsActivelyPressed = false;
}

void AMyButton::ServerOnRelease_Implementation()
{
    MulticastOnRelease();
}

void AMyButton::OnRelease()
{
    if (HasAuthority())
    {
        MulticastOnRelease();
    }
    else
    {
        ServerOnRelease();
    }
}

void AMyButton::MulticastOnDragOff_Implementation()
{
    OnButtonDraggedOff.Broadcast();
}

void AMyButton::ServerOnDragOff_Implementation()
{
    MulticastOnDragOff();
}

void AMyButton::OnDragOff()
{
    if (HasAuthority())
    {
        MulticastOnDragOff();
    }
    else
    {
        ServerOnDragOff();
    }
}
