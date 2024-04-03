#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h" // For random and vector math
#include "DrawDebugHelpers.h"        // For drawing debug shapes
#include "VertNeighb.generated.h"

UCLASS()
class BLASTER_API AVertNeighb : public AActor
{
	GENERATED_BODY()
	
public:	
	AVertNeighb();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spline")
	class USplineComponent* SplineComponent;

	// Spline mesh to be used along the spline
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spline")
	UStaticMesh* SplineMesh;

	// Scale of the mesh along the spline
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spline", meta=(ToolTip="Scale of the mesh along the spline. X=Along Spline, Y=Width, Z=Height"))
	FVector MeshScale;
private:
	FVector CurrentPosition; // Current position in the grid
	UPROPERTY(EditAnywhere)
	int32 GridSpacing = 1000;  // Space between grid locations
	void GenerateEnvironment(); // Function to generate the environment
	void CreateSplineMeshes();

};
