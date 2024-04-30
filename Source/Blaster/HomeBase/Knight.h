#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Animation/AnimInstance.h"
#include "Net/UnrealNetwork.h"
#include "SwordAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Knight.generated.h"

UCLASS()
class BLASTER_API AKnight : public AActor
{
	GENERATED_BODY()
    
public:    
	AKnight();

protected:
	virtual void BeginPlay() override;
	
public:
	virtual void Tick(float DeltaTime) override;
	void MulticastChangeSwordState_Implementation();

	UFUNCTION()
	void SwordButtonPressed();
	UFUNCTION()
	void SwordButtonDraggedOff();
	UFUNCTION()
	void SwordButtonReleased();

	UFUNCTION(BlueprintImplementableEvent)
	void BPSwordButtonPressed();
	UFUNCTION(BlueprintImplementableEvent)
	void BPSwordButtonDraggedOff();
	UFUNCTION(BlueprintImplementableEvent)
	void BPSwordButtonReleased();

	UPROPERTY(EditAnywhere, Category = "Components")
	UChildActorComponent* SwordButtonComponent;

	float SwordButtonHeldTime;

	UFUNCTION(BlueprintImplementableEvent)
	void SwordScreenShake(float ShakeIntensityUpToFour);
	
	UPROPERTY(BlueprintReadWrite)
	bool bSwordInserted = true;

	bool bSwordInteractable = true;

	FTimerHandle SwordTimeoutTimerHandle;

	UFUNCTION()
	void EndSwordInteractabilityTimeout();

	UFUNCTION(Server, Reliable)
	void ServerChangeSwordState();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastChangeSwordState();
	
	void ChangeSwordState();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USkeletalMeshComponent* KnightMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USkeletalMeshComponent* SwordMesh;

private:
	UPROPERTY(VisibleAnywhere, Category="Components")
	USceneComponent* RootComp;

	bool bButtonHeld = false;
public:
	void SetKnightAnimationTime(float MatchTime);
	UFUNCTION(BlueprintCallable)
	void TellSwordItsHeld();
	void ResetKnight();
	bool bIsAwake = false;
};
