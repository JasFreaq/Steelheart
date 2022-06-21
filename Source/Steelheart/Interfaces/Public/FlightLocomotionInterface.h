// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FlightLocomotionInterface.generated.h"

//DECLARE_DELEGATE_OneParam(FCharacterLandDelegate, const FHitResult&);

class UCameraComponent;
class UCapsuleComponent;
class UCharacterMovementComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UFlightLocomotionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class STEELHEART_API IFlightLocomotionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	//virtual FCharacterLandDelegate& GetCharacterLandDelegate() = 0;
	
	virtual UCameraComponent* GetCameraComponent() = 0;

	virtual UCapsuleComponent* GetCapsuleComponent() = 0;

	virtual UCharacterMovementComponent* GetCharacterMovementComponent() = 0;
};
