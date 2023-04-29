//so essentially you make this c++ class, then make a blueprint based on it
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

UCLASS()
class BLASTER_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	
	ACasing();

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CasingMesh;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
