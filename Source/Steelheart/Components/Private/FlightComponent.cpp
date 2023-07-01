#include "Steelheart/Components/Public/FlightComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Steelheart/Interfaces/Public/FlightLocomotionInterface.h"

// Sets default values for this component's properties
UFlightComponent::UFlightComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame. You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UFlightComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize the flight component
	InitializeFlightComponent();
}

void UFlightComponent::InitializeFlightComponent()
{
	// Get the owner character
	OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (ensure(OwnerCharacter != nullptr))
	{
		// Check if the owner character implements the flight locomotion interface
		FlightLocomotionInterface = Cast<IFlightLocomotionInterface>(OwnerCharacter);

		if (ensure(FlightLocomotionInterface != nullptr))
		{
			// Get the associated components from the owner character
			CameraComponent = FlightLocomotionInterface->GetCameraComponent();
			CapsuleComponent = OwnerCharacter->GetCapsuleComponent();
			CharacterMovement = OwnerCharacter->GetCharacterMovement();
		}
		else
		{
			// Log an error if the owner character does not implement the flight locomotion interface
			UE_LOG(LogTemp, Error, TEXT("FlightComponent owner does not implement FlightLocomotionInterface."));
		}
	}
	else
	{
		// Log an error if the owner is not a character
		UE_LOG(LogTemp, Error, TEXT("FlightComponent owner is not a character."));
	}
}
