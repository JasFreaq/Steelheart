#pragma once

#include "CoreMinimal.h"
#include "FlightComponent.h"
#include "FlightCollisionComponent.generated.h"

// Forward declarations
class UFieldSystemComponent;
class USphereComponent;
class URadialFalloff;
class URadialVector;
class UCullingField;

/**
 * Flight collision component responsible for handling collision events and generating collision effects.
 */
UCLASS(ClassGroup = (FlightLocomotion))
class STEELHEART_API UFlightCollisionComponent : public UFlightComponent
{
	GENERATED_BODY()

	// Field System Component used for collision effects
	UFieldSystemComponent* FieldSystem;

	// Collision sphere component for collision detection
	USphereComponent* CollisionSphere;

	// Radial falloff used for field properties
	URadialFalloff* RadialFalloff;

	// Radial vector used for field properties
	URadialVector* RadialVector;

	// Culling field used for field properties
	UCullingField* CullingField;

public:
	// Sets default values for this component's properties
	UFlightCollisionComponent();

	// Event called when the component hits another primitive component
	UFUNCTION()
		void OnCharacterHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:
	virtual void InitializeFlightComponent() override;

private:
	// Method to perform explosion effect
	void Explode();

	// Event called when the hit buffer timer expires
	UFUNCTION()
		void ResetHit();

	// Radius of the collision sphere
	UPROPERTY(EditDefaultsOnly, Category = CollisionParameters)
		float SphereRadius = 1800.f;

	// Magnitude of the falloff field
	UPROPERTY(EditDefaultsOnly, Category = CollisionParameters)
		float FalloffMagnitude = 500000.f;

	// Magnitude of the radial vector field
	UPROPERTY(EditDefaultsOnly, Category = CollisionParameters)
		float VectorMagnitude = 1000.f;

	// Tag to identify destructible objects
	UPROPERTY(EditDefaultsOnly, Category = CollisionParameters)
		FName DestructibleTag = "Destructible";

	// Time buffer to prevent multiple hits in a short interval
	UPROPERTY(EditDefaultsOnly, Category = CollisionParameters)
		float HitBufferTime = 0.8f;

	FTimerHandle HitBufferTimerHandle;
	FTimerDelegate HitBufferTimerDelegate;

	// Flag indicating if explosion can occur
	bool bCanExplode = true;
};
