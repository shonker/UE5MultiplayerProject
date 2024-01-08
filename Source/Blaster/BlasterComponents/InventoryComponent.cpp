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
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        UE_LOG(LogTemp, Log, TEXT("I love being the server"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("I love being NOOOOOOT the server"));
    }
    if (InventoryItems.Num() <= 1) return;

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

void UInventoryComponent::RemoveItem()
{
    if (!ActiveWeapon) return;

    InventoryItems.RemoveSingle(ActiveWeapon);
    ActiveWeapon = nullptr;
}

