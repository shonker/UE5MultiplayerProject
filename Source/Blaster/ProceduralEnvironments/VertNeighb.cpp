
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

	LastHorizontalDirection = FIntPoint(0, 0); // No initial direction
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
    
    FVector HorizontalDirections[] = {
        FVector(1, 0, 0),  // Forward
        FVector(0, 1, 0),  // Right
        FVector(-1, 0, 0), // Back
        FVector(0, -1, 0), // Left
    };
    
    FVector VerticalDirections[] = {
        FVector(1, 0, 1),  // Forward-Up
        FVector(1, 0, -1), // Forward-Down
        FVector(-1, 0, 1), // Back-Up
        FVector(-1, 0, -1),// Back-Down
        FVector(0, 1, 1),  // Right-Up
        FVector(0, 1, -1), // Right-Down
        FVector(0, -1, 1), // Left-Up
        FVector(0, -1, -1) // Left-Down
    };

    bool bMoveHorizontally = true;//(FMath::RandRange(0,10) < 1);
    for(int i = 0; i < 10; ++i)
    {
        DrawDebugSphere(GetWorld(), GetActorLocation() + CurrentPosition, 10.0f, 12, FColor::Blue, true);
        SplineComponent->AddSplinePoint(GetActorLocation() + CurrentPosition, ESplineCoordinateSpace::World, true);

        FVector ChosenDirection = FVector::ZeroVector;
    	FVector NextMovement = FVector::ZeroVector;
        bool bValidMoveFound = false;

        while (!bValidMoveFound)
        {
            if (bMoveHorizontally)
            {
                int32 NumDirections = sizeof(HorizontalDirections) / sizeof(FVector);
                ChosenDirection = HorizontalDirections[FMath::RandRange(0, NumDirections - 1)];
            }
            else
            {
                int32 NumDirections = sizeof(VerticalDirections) / sizeof(FVector);
                ChosenDirection = VerticalDirections[FMath::RandRange(0, NumDirections - 1)];
            }
        	NextMovement = ChosenDirection * GridSpacing;
        	if (!bMoveHorizontally)
        	{
        		NextMovement.X*=1.5;
        		NextMovement.Y*=1.5;
        	}
        	
            FIntPoint CurrentHorizontalMove(int32(FMath::Sign(ChosenDirection.X)), int32(FMath::Sign(ChosenDirection.Y)));

            if (CurrentHorizontalMove != ForbiddenDirection || CurrentHorizontalMove == LastHorizontalDirection)
            {
            	UE_LOG(LogTemp, Warning, TEXT("Last Direction: (%d, %d), Current Direction: (%d, %d), Forbidden Direction: (%d, %d)"), 
			   LastHorizontalDirection.X, LastHorizontalDirection.Y, 
			   CurrentHorizontalMove.X, CurrentHorizontalMove.Y,
			   ForbiddenDirection.X,ForbiddenDirection.Y);
            	
                // Convert the next position to a grid location to check if it's been visited
                FVector NextPosition = CurrentPosition + NextMovement;
                FIntVector NextGridLocation = FIntVector(FMath::RoundToInt(NextPosition.X / GridSpacing), FMath::RoundToInt(NextPosition.Y / GridSpacing), FMath::RoundToInt(NextPosition.Z / GridSpacing));

                if (!VisitedLocations.Contains(NextGridLocation) || !VisitedLocations[NextGridLocation])
                {
                    VisitedLocations.Add(NextGridLocation, true);
                	
                    LastHorizontalDirection = CurrentHorizontalMove;
                    ForbiddenDirection = CurrentHorizontalMove * -1;
                    bValidMoveFound = true;
                }
            }
            else
            {
	            UE_LOG(LogTemp, Warning, TEXT("Rejected due to doubling back. Last Direction: (%d, %d), Current Direction: (%d, %d), Forbidden Direction: (%d, %d)"), 
			   LastHorizontalDirection.X, LastHorizontalDirection.Y, 
			   CurrentHorizontalMove.X, CurrentHorizontalMove.Y,
			   ForbiddenDirection.X,ForbiddenDirection.Y);
            }
        }

        if (bValidMoveFound)
        {
            DrawDebugLine(GetWorld(), GetActorLocation() + CurrentPosition, GetActorLocation() + CurrentPosition + NextMovement, FColor::Green, true, -1, 0, 2);
            CurrentPosition += NextMovement;
        }
        
        // Toggle movement type for next iteration
        bMoveHorizontally = !bMoveHorizontally;
    }

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
