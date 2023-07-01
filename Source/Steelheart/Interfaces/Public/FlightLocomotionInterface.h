// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FlightLocomotionInterface.generated.h"

class UCameraComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UFlightLocomotionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * The FlightLocomotionInterface is an interface class that provides functions for flight locomotion behavior.
 */
class STEELHEART_API IFlightLocomotionInterface
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * Retrieves the camera component associated with the flight locomotion.
	 *
	 * @return The camera component.
	 */
	virtual UCameraComponent* GetCameraComponent() = 0;

	/**
	 * Checks if the character is currently dashing.
	 *
	 * @return True if the character is dashing, false otherwise.
	 */
	virtual bool IsDashing() = 0;

	/**
	 * Sets the locomotion state for the character.
	 *
	 * @param Enabled - True to enable locomotion, false to disable locomotion.
	 */
	virtual void SetLocomotionEnabled(bool Enabled) = 0;
};