// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlightComponent.generated.h"

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
		
	ACharacter* OwnerCharacter = nullptr;

	IFlightLocomotionInterface* FlightLocomotionInterface = nullptr;

	UCameraComponent* CameraComponent = nullptr;

	UCapsuleComponent* CapsuleComponent = nullptr;

	UCharacterMovementComponent* CharacterMovement = nullptr;

	template<typename T>
	T* SetupAssociatedComponent(USceneComponent* ParentComp)
	{
		UActorComponent* NewComp = GetOwner()->AddComponentByClass(T::StaticClass(), true,
			GetOwner()->GetActorTransform(), true);

		if (T* TypedComp = Cast<T>(NewComp))
		{
			if (USceneComponent* SceneComp = Cast<USceneComponent>(NewComp))
			{
				SceneComp->SetupAttachment(ParentComp);
			}

			NewComp->SetAutoActivate(false);
			NewComp->RegisterComponent();

			return TypedComp;
		}

		return nullptr;
	}
};
