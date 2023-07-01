#include "Steelheart/Components/Public/FlightCollisionComponent.h"
#include "Components/SphereComponent.h"
#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemObjects.h"
#include "GameFramework/Character.h"
#include "Steelheart/Interfaces/Public/FlightLocomotionInterface.h"

// Sets default values for this component's properties
UFlightCollisionComponent::UFlightCollisionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.
	// You can turn these features off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// Bind the ResetHit function to the HitBufferTimerDelegate
	HitBufferTimerDelegate.BindUFunction(this, "ResetHit");
}

void UFlightCollisionComponent::OnCharacterHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bCanExplode && FlightLocomotionInterface->IsDashing() && OtherActor->ActorHasTag(DestructibleTag))
	{
		Explode();

		bCanExplode = false;
		GetWorld()->GetTimerManager().SetTimer(HitBufferTimerHandle, HitBufferTimerDelegate, HitBufferTime, false);
	}
}

void UFlightCollisionComponent::InitializeFlightComponent()
{
	Super::InitializeFlightComponent();

	// Setup the FieldSystemComponent associated with the owning character's root component
	FieldSystem = SetupAssociatedComponent<UFieldSystemComponent>(OwnerCharacter->GetRootComponent());

	// Setup the CollisionSphere component associated with the FieldSystemComponent
	CollisionSphere = SetupAssociatedComponent<USphereComponent>(FieldSystem);
	CollisionSphere->SetSphereRadius(SphereRadius);

	// Setup the RadialFalloff component
	RadialFalloff = SetupAssociatedComponent<URadialFalloff>(nullptr);

	// Setup the RadialVector component
	RadialVector = SetupAssociatedComponent<URadialVector>(nullptr);

	// Setup the CullingField component
	CullingField = SetupAssociatedComponent<UCullingField>(nullptr);
}

void UFlightCollisionComponent::Explode()
{
	// Get the location of the CollisionSphere component
	FVector SphereLoc = CollisionSphere->GetComponentLocation();

	// Set up the RadialFalloff field properties
	UFieldNodeBase* RadialFalloffNode = RadialFalloff->SetRadialFalloff(FalloffMagnitude, 0.f, 1.f, 0.f,
		CollisionSphere->GetScaledSphereRadius(), SphereLoc, Field_FallOff_None);

	// Apply the RadialFalloff field to the FieldSystemComponent
	FieldSystem->ApplyPhysicsField(true, Field_ExternalClusterStrain, nullptr, RadialFalloffNode);

	// Set up the RadialVector field properties
	UFieldNodeBase* RadialVectorNode = RadialVector->SetRadialVector(VectorMagnitude, SphereLoc);

	// Set up the CullingField properties using the RadialFalloff and RadialVector nodes
	UFieldNodeBase* CullingFieldNode = CullingField->SetCullingField(RadialFalloffNode, RadialVectorNode, Field_Culling_Outside);

	// Apply the CullingField to the FieldSystemComponent
	FieldSystem->ApplyPhysicsField(true, Field_LinearVelocity, nullptr, CullingFieldNode);
}

void UFlightCollisionComponent::ResetHit()
{
	// Reset the flag to allow future explosions
	bCanExplode = true;

	// Clear the HitBufferTimerHandle
	GetWorld()->GetTimerManager().ClearTimer(HitBufferTimerHandle);
}
