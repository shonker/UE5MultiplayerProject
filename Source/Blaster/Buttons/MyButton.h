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
protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UBoxComponent* AreaBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UWidgetComponent* InteractWidget;

public:
    virtual void Tick(float DeltaTime) override;

    // Overlap functions
    UFUNCTION()
        void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
        void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Button-specific functions
    virtual void OnInitPress();
    virtual void WhileHeld();
    virtual void OnRelease();
    AActor* OwningActor;
private:
    bool IsActivelyPressed = false;
};
