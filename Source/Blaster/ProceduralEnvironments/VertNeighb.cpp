
#include "VertNeighb.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

AVertNeighb::AVertNeighb()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentPosition = FVector::ZeroVector;
	USceneComponent* StaticRoot = CreateDefaultSubobject<USceneComponent>(TEXT("StaticRoot"));
	RootComponent = StaticRoot;
	RootComponent->SetMobility(EComponentMobility::Movable); // Set the root component mobility to Static

	// Initialize the SplineComponent and attach it to the static root
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	SplineComponent->SetupAttachment(RootComponent);
	SplineComponent->SetMobility(EComponentMobility::Movable);

}

void AVertNeighb::BeginPlay()
{
	Super::BeginPlay();
	GenerateEnvironment();
	CreateSplineMeshes();
}

void AVertNeighb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AVertNeighb::GenerateEnvironment()
{
	if (!SplineComponent) return;
	FVector Directions[] = {
		FVector(1, 0, 0),  // Forward
		FVector(0, 1, 0),  // Right
		FVector(-1, 0, 0), // Back
		FVector(0, -1, 0), // Left
		//FVector(0, 0, 1),  // Up
		//FVector(0, 0, -1), // Down
		FVector(1, 0, 1),  // Forward-Up
		FVector(1, 0, -1), // Forward-Down
		FVector(-1, 0, 1), // Back-Up
		FVector(-1, 0, -1),// Back-Down
		FVector(0, 1, 1),  // Right-Up
		FVector(0, 1, -1), // Right-Down
		FVector(0, -1, 1), // Left-Up
		FVector(0, -1, -1) // Left-Down
	};

	for(int i = 0; i < 10; ++i)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation() + CurrentPosition, 10.0f, 12, FColor::Blue, true);
		SplineComponent->AddSplinePoint(GetActorLocation() + CurrentPosition, ESplineCoordinateSpace::World, true);

		int32 num_directions = sizeof(Directions) / sizeof(FVector);
		FVector ChosenDirection = Directions[FMath::RandRange(0, num_directions - 1)] * GridSpacing;
		
		DrawDebugLine(GetWorld(), GetActorLocation() + CurrentPosition, GetActorLocation() + CurrentPosition + ChosenDirection, FColor::Green, true, -1, 0, 2);
		CurrentPosition += ChosenDirection;

	}

	// Update the spline to visualize the changes
	SplineComponent->UpdateSpline();
}

void AVertNeighb::CreateSplineMeshes()
{

	if (!SplineMesh) return; // Ensure there's a mesh selected
	const int32 NumSegments = SplineComponent->GetNumberOfSplinePoints() - 1;

	for (int32 i = 0; i < NumSegments; ++i)
	{
		USplineMeshComponent* SplineMeshComponent = NewObject<USplineMeshComponent>(this);
		SplineMeshComponent->SetMobility(EComponentMobility::Movable); // Ensure mobility is set to Movable
		if (!SplineMeshComponent) continue;

		SplineMeshComponent->RegisterComponent();
		SplineMeshComponent->AttachToComponent(SplineComponent, FAttachmentTransformRules::KeepWorldTransform);
		SplineMeshComponent->SetStaticMesh(SplineMesh);
		SplineMeshComponent->SetWorldScale3D(MeshScale);
		SplineMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SplineMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

		// Setup start and end points
		FVector StartPos, StartTangent, EndPos, EndTangent;
		SplineComponent->GetLocationAndTangentAtSplinePoint(i, StartPos, StartTangent, ESplineCoordinateSpace::Local);
		SplineComponent->GetLocationAndTangentAtSplinePoint(i + 1, EndPos, EndTangent, ESplineCoordinateSpace::Local);

		// Apply the scale to tangents to control the curvature
		StartTangent *= MeshScale.X;
		EndTangent *= MeshScale.X;

		SplineMeshComponent->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent, true);

		// Optional: Setup material, collision, etc.
	}
}
