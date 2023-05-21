// Fill out your copyright notice in the Description page of Project Settings.


#include "Steelheart/Components/Public/FlightCollisionComponent.h"
#include "Components/SphereComponent.h"
#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemObjects.h"
#include "GameFramework/Character.h"

// Sets default values for this component's properties
UFlightCollisionComponent::UFlightCollisionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

void UFlightCollisionComponent::Explode()
{
	FVector SphereLoc = CollisionSphere->GetComponentLocation();

	UFieldNodeBase* RadialFalloffNode = RadialFalloff->SetRadialFalloff(FalloffMagnitude, 0.f, 1.f, 0.f,
		CollisionSphere->GetScaledSphereRadius(), SphereLoc, Field_FallOff_None);

	FieldSystem->ApplyPhysicsField(true, Field_ExternalClusterStrain, nullptr, RadialFalloffNode);

	UFieldNodeBase* RadialVectorNode = RadialVector->SetRadialVector(VectorMagnitude, SphereLoc);

	UFieldNodeBase* CullingFieldNode = CullingField->SetCullingField(RadialFalloffNode, RadialVectorNode, Field_Culling_Outside);

	FieldSystem->ApplyPhysicsField(true, Field_LinearVelocity, nullptr, CullingFieldNode);
}
