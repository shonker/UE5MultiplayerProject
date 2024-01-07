#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

// Forward declaration
class AWeapon;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLASTER_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Inventory management
    void EquipItem(AWeapon* Weapon);
    void ShuffleItem(bool bIsShuffleLeft);
    void ThrowItem();

    // Replication
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(Replicated)
        TArray<AWeapon*> InventoryItems;

    UPROPERTY(Replicated)
        AWeapon* ActiveWeapon;
};
