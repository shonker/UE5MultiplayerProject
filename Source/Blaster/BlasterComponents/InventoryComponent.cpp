#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"
#include "GameFramework/Actor.h"
#include "Blaster/BlasterComponents/CombatComponent.h"


UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UInventoryComponent, InventoryItems);
    DOREPLIFETIME(UInventoryComponent, ActiveWeapon);
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

//should only be called in server
void UInventoryComponent::AddItem(AWeapon* Weapon)
{
    if (!Weapon) return;    

    InventoryItems.Add(Weapon);

    if (Combat)
    {
    	Combat->EquipWeapon(Weapon);
    }
    ActiveWeapon = Weapon;
}

void UInventoryComponent::ShuffleItem(bool bIsShuffleLeft)
{
    if (InventoryItems.Num() <= 1) return;

    int32 CurrentIndex = InventoryItems.IndexOfByKey(ActiveWeapon);
    int32 NextIndex;

    if (bIsShuffleLeft)
    {
        NextIndex = (CurrentIndex - 1 + InventoryItems.Num()) % InventoryItems.Num();
    }
    else
    {
        NextIndex = (CurrentIndex + 1) % InventoryItems.Num();
    }

    ActiveWeapon = InventoryItems[NextIndex];

    if (Combat)
    {
        Combat->EquipWeapon(ActiveWeapon);
    }
}

void UInventoryComponent::RemoveAllItems()
{
    for (int32 i = 0; i < InventoryItems.Num(); i++)
    {
        if (!InventoryItems[i]) continue;
        UE_LOG(LogTemp, Log, TEXT("dropping weapons"));
        InventoryItems[i]->Dropped();

        if (!InventoryItems[i]->GetWeaponMesh()) continue;
        float RandX = FMath::RandRange(-10.f, 10.f);
        float RandY = FMath::RandRange(-10.f, 10.f);
        float RandZ = FMath::RandRange(0.f, 10.f);
        FVector DropImpulse = FVector(RandX, RandY, RandZ) * 10;
        InventoryItems[i]->GetWeaponMesh()->AddImpulse(DropImpulse, NAME_None, true);
    }
    InventoryItems.Empty();
}

void UInventoryComponent::RemoveItem()
{
    if (!ActiveWeapon) return;

    InventoryItems.RemoveSingle(ActiveWeapon);
    ActiveWeapon = nullptr;
}

