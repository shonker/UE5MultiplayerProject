#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "MyButton.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FButtonPressedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FButtonReleasedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FButtonHeldDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FButtonDraggedOffDelegate);

UCLASS()
class BLASTER_API AMyButton : public AActor
{
    GENERATED_BODY()

public:
    AMyButton();
    // Event dispatchers
    UPROPERTY(BlueprintAssignable, Category = "Button")
        FButtonPressedDelegate OnButtonPressed;

    UPROPERTY(BlueprintAssignable, Category = "Button")
        FButtonReleasedDelegate OnButtonReleased;

    UPROPERTY(BlueprintAssignable, Category = "Button")
        FButtonHeldDelegate OnButtonHeld;
    
    UPROPERTY(BlueprintAssignable, Category = "Button")
        FButtonHeldDelegate OnButtonDraggedOff;

    UPROPERTY(EditAnywhere , Category = "Button")
    FString InteractionText{ TEXT("???") };

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UBoxComponent* AreaBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UWidgetComponent* InteractWidget;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
        void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
        void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(NetMulticast, Reliable)
        virtual void MulticastOnInitPress();
    UFUNCTION(Server, Reliable)
        virtual void ServerOnInitPress();
    virtual void OnInitPress();
    
    UFUNCTION(NetMulticast, Unreliable)
        virtual void MulticastWhileHeld();
    UFUNCTION(Server, Unreliable)
        virtual void ServerWhileHeld();
    virtual void WhileHeld();
   
    UFUNCTION(NetMulticast, Reliable)
        virtual void MulticastOnRelease();
    UFUNCTION(Server, Reliable)
        virtual void ServerOnRelease();
    virtual void OnRelease();

    UFUNCTION(NetMulticast, Reliable)
        virtual void MulticastOnDragOff();
    UFUNCTION(Server, Reliable)
        virtual void ServerOnDragOff();
    virtual void OnDragOff();
    AActor* OwningActor;
private:
    
    bool IsActivelyPressed = false;
};
