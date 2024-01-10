#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "MyButton.generated.h"

UCLASS()
class BLASTER_API AMyButton : public AActor
{
    GENERATED_BODY()

public:
    AMyButton();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UBoxComponent* AreaBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UWidgetComponent* PickupWidget;

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

private:
    bool IsActivelyPressed = false;
};
