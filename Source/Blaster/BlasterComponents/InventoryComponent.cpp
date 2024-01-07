#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h" // Assuming Weapon class exists

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UInventoryComponent::EquipItem(AWeapon* Weapon)
{
    if (!Weapon) return;

    // Check if there is an active weapon, if so, store it in inventory
    if (ActiveWeapon)
    {
        InventoryItems.Add(ActiveWeapon);
    }

    // Set the new weapon as the active weapon
    ActiveWeapon = Weapon;

    // TODO: Additional logic for equipping the weapon visually, etc.
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

    // TODO: Additional logic for updating the weapon visually, etc.
}

void UInventoryComponent::ThrowItem()
{
    if (!ActiveWeapon) return;

    // TODO: Implement the logic to 'throw' the weapon

    // Remove the active weapon from the inventory
    InventoryItems.RemoveSingle(ActiveWeapon);
    ActiveWeapon = nullptr;
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UInventoryComponent, InventoryItems);
    DOREPLIFETIME(UInventoryComponent, ActiveWeapon);
}
