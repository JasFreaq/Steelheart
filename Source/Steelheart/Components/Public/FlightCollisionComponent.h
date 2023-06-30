// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlightComponent.h"
#include "FlightCollisionComponent.generated.h"

class UFieldSystemComponent;
class USphereComponent;
class URadialFalloff;
class URadialVector;
class UCullingField;

/**
 * 
 */
UCLASS(ClassGroup = (FlightLocomotion))
class STEELHEART_API UFlightCollisionComponent : public UFlightComponent
{
	GENERATED_BODY()

	UFieldSystemComponent* FieldSystem;

	USphereComponent* CollisionSphere;

	URadialFalloff* RadialFalloff;

	URadialVector* RadialVector;

	UCullingField* CullingField;

public:
	// Sets default values for this component's properties
	UFlightCollisionComponent();

	UFUNCTION()
		void OnCharacterHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:
	virtual void InitializeFlightComponent() override;

private:
	void Explode();

	UFUNCTION()
		void ResetHit();

	UPROPERTY(EditDefaultsOnly, Category = CollisionParameters)
		float SphereRadius = 1800.f;

	UPROPERTY(EditDefaultsOnly, Category = CollisionParameters)
		float FalloffMagnitude = 500000.f;

	UPROPERTY(EditDefaultsOnly, Category = CollisionParameters)
		float VectorMagnitude = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = CollisionParameters)
		FName DestructibleTag = "Destructible";

	UPROPERTY(EditDefaultsOnly, Category = CollisionParameters)
		float HitBufferTime = 0.8f;

	FTimerHandle HitBufferTimerHandle;

	FTimerDelegate HitBufferTimerDelegate;

	bool bCanExplode = true;
};
