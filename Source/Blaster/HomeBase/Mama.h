#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Mama.generated.h"

UENUM(BlueprintType)
enum class EState : uint8
{
	IDLE UMETA(DisplayName = "Idle"),
	ACTIVATING UMETA(DisplayName = "Activating"),
	CONSUMING UMETA(DisplayName = "Consuming"),
	SATED UMETA(DisplayName = "Sated"),
	STARVED UMETA(DisplayName = "Starved")
};

UCLASS()
class BLASTER_API AMama : public AActor
{
	GENERATED_BODY()

public:
	AMama();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void BeginActivatingState();

	UFUNCTION(BlueprintCallable)
	void BeginConsumingState();



protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(ReplicatedUsing = OnRep_State, BlueprintReadOnly, Category = "State")
		EState State = EState::IDLE;

	UFUNCTION()
		void OnRep_State();

	UFUNCTION(NetMulticast, Reliable)
		void Multi_PlayEffectsForWeapon(const FVector& Location);

	UFUNCTION(NetMulticast, Reliable)
		void Multi_PlayEffectsForCharacter(const FVector& Location);

	UFUNCTION()
	void HandleCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		class USkeletalMeshComponent* MamaHead;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		class UCapsuleComponent* InvisBarrier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		UCapsuleComponent* Consumer;


	UPROPERTY(VisibleAnywhere, Category = "Curse")
		int32 TotalCurseTally = 0;
	UPROPERTY(EditAnywhere, Category = "Curse")
		int32 TargetCurseTally = 10;
	/*
		SFX VFX
	*/
	UPROPERTY(EditAnywhere, Category = "Effects")
		class USoundCue* LoopingHumSFX;

	UPROPERTY(EditAnywhere, Category = "Effects")
		USoundCue* ItemConsumedSFX;

	UPROPERTY(EditAnywhere, Category = "Effects")
		USoundCue* CharConsumedSFX;

	UPROPERTY(EditAnywhere, Category = "Effects")
		class UParticleSystem* ItemConsumedVFX;

	UPROPERTY(EditAnywhere, Category = "Effects")
		UParticleSystem* CharConsumedVFX;

	FTimerHandle StateTimerHandle;

	void ChangeState(EState NewState);
	void EvaluateConsumedMaterials();
	
	class AHomeBase* HomeBase;
	 
public:
	void SetHomeBase(AHomeBase* HB);
};
