#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"
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
    
    //todo: store ActiveWeapon

    InventoryItems.Add(Weapon);

    if (Combat)
    {
    	Combat->EquipWeapon(Weapon);
    }
}

void UInventoryComponent::ShuffleItem(bool bIsShuffleLeft)
{
    if (InventoryItems.Num() == 0) return;

    // Find the index of the current active weapon
    int32 CurrentIndex = InventoryItems.IndexOfByKey(ActiveWeapon);
    int32 NextIndex;

    // Determine the next index based on the shuffle direction
    if (bIsShuffleLeft)
    {
        NextIndex = (CurrentIndex - 1 + InventoryItems.Num()) % InventoryItems.Num();
    }
    else
    {
        NextIndex = (CurrentIndex + 1) % InventoryItems.Num();
    }

    // Update the active weapon
    ActiveWeapon = InventoryItems[NextIndex];

    if (Combat)
    {
        Combat->EquipWeapon(ActiveWeapon);
    }
}

void UInventoryComponent::ThrowItem()
{
    if (!ActiveWeapon) return;

    // TODO: Implement the logic to 'throw' the weapon

    // Remove the active weapon from the inventory
    InventoryItems.RemoveSingle(ActiveWeapon);
    ActiveWeapon = nullptr;
}

