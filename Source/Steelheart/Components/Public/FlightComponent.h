// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlightComponent.generated.h"

// Forward declarations
class IFlightLocomotionInterface;
class UCameraComponent;
class UCapsuleComponent;
class UCharacterMovementComponent;

UCLASS(Abstract, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class STEELHEART_API UFlightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFlightComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void InitializeFlightComponent();

	// Owner character of the flight component
	ACharacter* OwnerCharacter = nullptr;

	// Interface for flight locomotion
	IFlightLocomotionInterface* FlightLocomotionInterface = nullptr;

	// Camera component associated with the owner character
	UCameraComponent* CameraComponent = nullptr;

	// Capsule component associated with the owner character
	UCapsuleComponent* CapsuleComponent = nullptr;

	// Character movement component associated with the owner character
	UCharacterMovementComponent* CharacterMovement = nullptr;

	// Template function to set up an associated component
	template<typename T>
	T* SetupAssociatedComponent(USceneComponent* ParentComp)
	{
		// Add the component of type T to the owner character
		UActorComponent* NewComp = GetOwner()->AddComponentByClass(T::StaticClass(), true,
			GetOwner()->GetActorTransform(), true);

		if (T* TypedComp = Cast<T>(NewComp))
		{
			if (USceneComponent* SceneComp = Cast<USceneComponent>(NewComp))
			{
				// Attach the scene component to the provided parent component
				SceneComp->SetupAttachment(ParentComp);
			}

			// Disable auto activation of the new component
			NewComp->SetAutoActivate(false);

			// Register the new component
			NewComp->RegisterComponent();

			return TypedComp;
		}

		return nullptr;
	}
};