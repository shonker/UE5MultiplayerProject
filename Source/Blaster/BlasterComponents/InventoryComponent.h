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
    void AddItem(AWeapon* Weapon);
    void ShuffleItem(bool bIsShuffleLeft);
    void RemoveItem();
    void RemoveAllItems();

    
    // Replication
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    //meta specifier allows private variables to be blueprintreadonly
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
        class UCombatComponent* Combat;
private:
    UPROPERTY(Replicated)
        TArray<AWeapon*> InventoryItems;

    UPROPERTY(Replicated)
        AWeapon* ActiveWeapon;
    
    UPROPERTY()
        int32 InventoryItemLimit = 6;

    
};
