
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
    //AProcPark();

protected:
    virtual void BeginPlay() override;

private:
    void SpawnNextObject(const FTransform& ParentTransform, int32 CurrentLifetime, int32 CurrentBranchCount);

    UPROPERTY(EditAnywhere, Category = "Procedural Generation")
        TArray<FObjectTypeInfo> ObjectTypes;

    void AdjustRotationAndSpawn(const FTransform& WorldTransform, const FObjectTypeInfo& SelectedType, int32 CurrentLifetime, int32 CurrentBranchCount);

    FVector CalculateOutputLocation(const AActor* SpawnedObject, const FVector& OutputLocation, const FRotator& Rotation);

    FObjectTypeInfo GetRandomObjectTypeExcluding(const TSubclassOf<AProcParkPart>& ExcludeType);
    TSubclassOf<AProcParkPart> LastSpawnedParkPart = nullptr;

    const int32 MaxBranchCount = 4;
    int32 BranchCount = 0;


};

