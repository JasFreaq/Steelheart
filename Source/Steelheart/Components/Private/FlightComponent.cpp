// Fill out your copyright notice in the Description page of Project Settings.


#include "Steelheart/Components/Public/FlightComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Steelheart/Interfaces/Public/FlightLocomotionInterface.h"

// Sets default values for this component's properties
UFlightComponent::UFlightComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
}

// Called when the game starts
void UFlightComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeFlightComponent();
}

void UFlightComponent::InitializeFlightComponent()
{
	OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (ensure(OwnerCharacter != nullptr))
	{
		FlightLocomotionInterface = Cast<IFlightLocomotionInterface>(OwnerCharacter);

		if (ensure(FlightLocomotionInterface != nullptr))
		{
			CameraComponent = FlightLocomotionInterface->GetCameraComponent();
			CapsuleComponent = OwnerCharacter->GetCapsuleComponent();
			CharacterMovement = OwnerCharacter->GetCharacterMovement();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("FlightComponent owner does not implement FlightLocomotionInterface."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FlightComponent owner is not a character."));
	}
}