
#pragma once

#include "CoreMinimal.h"
#include "Blaster/ProceduralEnvironments/AProcActor.h"
#include "Components/BoxComponent.h" // Include the BoxComponent header
#include "Mama.h"
#include "HomeBase.generated.h"

UCLASS()
class BLASTER_API AHomeBase : public AAProcActor
{
	GENERATED_BODY()

public:
	// Constructor to set default values and initialize components
	AHomeBase();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Box component to detect player overlap
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UBoxComponent* OverlapComponent;

	// Function to be called when players overlap the box
	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Function to signal the game mode to start the match countdown
	void StartMatchCountdown();

	// Function called at the end of the match countdown
	UFUNCTION(BlueprintCallable)
		void BeginJudgement();
	UPROPERTY(ReplicatedUsing = OnRep_ActivateLights, BlueprintReadOnly)
		bool bLights;
	UPROPERTY(ReplicatedUsing = OnRep_RedLights, BlueprintReadOnly)
		bool bRedLights;
	UFUNCTION()
		void OnRep_RedLights();
	UFUNCTION()
		void OnRep_ActivateLights();
	UFUNCTION(BlueprintImplementableEvent, Category = "Gameplay")
		void RedLights();
	UFUNCTION(BlueprintImplementableEvent, Category = "Gameplay")
		void ActivateLights();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
		AMama* MamaActor;
	void ActivateMama();
	void MamaStarved();

};
