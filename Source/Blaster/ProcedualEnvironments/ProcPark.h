
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "ProcParkPart.h"  
#include "ProcPark.generated.h"



USTRUCT(BlueprintType)
struct FObjectTypeInfo
{
    GENERATED_BODY()

        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TSubclassOf<AProcParkPart> ObjectBlueprint; 
};

UCLASS()
class BLASTER_API AProcPark : public AActor
{
    GENERATED_BODY()

public:
    AProcPark();

protected:
    virtual void BeginPlay() override;

private:
    void SpawnNextObject(const FVector& Location, const FRotator& Rotation, int32 CurrentLifetime, int32 CurrentBranchCount);

    UPROPERTY(EditAnywhere, Category = "Procedural Generation")
        TArray<FObjectTypeInfo> ObjectTypes;

    UPROPERTY(EditAnywhere, Category = "Procedural Generation")
        TSubclassOf<AActor> StaircaseBlueprint;

    UPROPERTY(EditAnywhere, Category = "Procedural Generation")
        TSubclassOf<AActor> SlideBlueprint;

    FVector CalculateOutputLocation(const AActor* SpawnedObject, const FVector& OutputLocation, const FRotator& Rotation);

    FObjectTypeInfo GetRandomObjectTypeExcluding(const TSubclassOf<AProcParkPart>& ExcludeType);
    TSubclassOf<AProcParkPart> LastSpawnedParkPart = nullptr;

    const int32 MaxBranchCount = 3;


};

