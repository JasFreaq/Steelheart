// Fill out your copyright notice in the Description page of Project Settings.

#include "Steelheart/Components/Public/FlightLocomotionComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Steelheart/Interfaces/Public/FlightLocomotionInterface.h"

// Sets default values for this component's properties
UFlightLocomotionComponent::UFlightLocomotionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.
	// You can turn these features off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// Binding delegates for dodge and divebomb functionality
	DodgeTimerDelegate.BindUFunction(this, "ResetDodge");
	DodgeResetBufferTimerDelegate.BindUFunction(this, "ResetDodgeTimer");
	DivebombTimerDelegate.BindUFunction(this, "InitiateDivebomb");
	DivebombLandTimerDelegate.BindUFunction(this, "EndDivebombLand");
}

//////////////////////////////////////////////////////////////////////////
// Lifecycle Functions

void UFlightLocomotionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Retrieve the half height of the capsule component
	CapsuleHalfHeight = CapsuleComponent->GetUnscaledCapsuleHalfHeight();

	// Set character movement properties
	CharacterMovement->MaxAcceleration = BaseAcceleration;
	CharacterMovement->BrakingDecelerationFlying = BrakingDecelerationFlying;

	// Calculate divebomb start and land section lengths based on montage
	if (ensure(DivebombMontage != nullptr))
	{
		int32 StartSectionIndex = DivebombMontage->GetSectionIndex("Default");
		DivebombStartSectionLength = DivebombMontage->GetSectionLength(StartSectionIndex) / DIVEBOMB_RATE_SCALE;

		int32 LandSectionIndex = DivebombMontage->GetSectionIndex(DiveMontageLandSectionName);
		DivebombLandSectionLength = DivebombMontage->GetSectionLength(LandSectionIndex) / DIVEBOMB_RATE_SCALE;
	}

	// Set trace parameters for divebomb
	DivebombTraceParams.AddIgnoredActor(OwnerCharacter);
	DivebombTraceParams.bTraceComplex = true;
}

// Called every frame
void UFlightLocomotionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Check if the character is flying
	if (CharacterMovement->IsFlying())
	{
		UpdateFlightLocomotion(DeltaTime);
	}
	// Check if the character is falling
	else if (CharacterMovement->IsFalling())
	{
		// Check if divebomb was initiated
		if (bInitiatedDivebomb)
		{
			if (bIsDivebombing)
			{
				UpdateDivebomb(DeltaTime);
			}
		}
		else
		{
			InitiateDivebombStart();
		}
	}

	// Check if the character was dashing
	if (bWasDashing)
	{
		SmoothResetPitch(DeltaTime);
	}
}

void UFlightLocomotionComponent::Fly()
{
	// Set character movement properties for flight
	CharacterMovement->MaxFlySpeed = BaseSpeed;
	CharacterMovement->MaxAcceleration = BaseAcceleration;

	CharacterMovement->SetMovementMode(MOVE_Flying);
}

void UFlightLocomotionComponent::StopFlying()
{
	// Store the landing initiation location
	LandingInitiationLocationZ = OwnerCharacter->GetActorLocation().Z;

	// Calculate the Z momentum and launch the character accordingly
	FVector CapsuleLinearVelocity = CapsuleComponent->GetPhysicsLinearVelocity();
	float ZMomentum = CapsuleLinearVelocity.Z;

	FVector LaunchVelocity(0.f, 0.f, ZMomentum * ZMomentumCoeff);
	OwnerCharacter->LaunchCharacter(LaunchVelocity, false, true);

	// Set movement mode to falling and restore capsule half height
	CharacterMovement->SetMovementMode(MOVE_Falling);
	CapsuleComponent->SetCapsuleHalfHeight(CapsuleHalfHeight);
}

void UFlightLocomotionComponent::Dash()
{
	// Set character movement properties for dashing
	CharacterMovement->MaxFlySpeed = DashSpeed;
	CharacterMovement->MaxAcceleration = DashAcceleration;

	// Adjust capsule half height
	CapsuleComponent->SetCapsuleHalfHeight(CapsuleComponent->GetUnscaledCapsuleRadius());

	bWasDashing = false;
}

void UFlightLocomotionComponent::StopDashing()
{
	// Reset character movement properties after dashing
	CharacterMovement->MaxFlySpeed = BaseSpeed;
	CharacterMovement->MaxAcceleration = BaseAcceleration;

	// Restore capsule half height
	CapsuleComponent->SetCapsuleHalfHeight(CapsuleHalfHeight);

	bIsDodgingRight = false;
	bIsDodgingLeft = false;

	bWasDashing = true;
}

void UFlightLocomotionComponent::RightDodge()
{
	// Check if the character is dashing and not currently dodging in any direction
	if (FlightLocomotionInterface->IsDashing() && !bIsDodging && !(bIsDodgingRight || bIsDodgingLeft))
	{
		bIsDodging = true;
		bIsDodgingRight = true;

		CurrentDodgeForce = BaseDodgeForce;

		// Set a timer to reset the dodge state
		GetWorld()->GetTimerManager().SetTimer(DodgeTimerHandle, DodgeTimerDelegate, DodgeTime, false);
	}
}

void UFlightLocomotionComponent::LeftDodge()
{
	// Check if the character is dashing and not currently dodging in any direction
	if (FlightLocomotionInterface->IsDashing() && !bIsDodging && !(bIsDodgingRight || bIsDodgingLeft))
	{
		bIsDodging = true;
		bIsDodgingLeft = true;

		CurrentDodgeForce = BaseDodgeForce;

		// Set a timer to reset the dodge state
		GetWorld()->GetTimerManager().SetTimer(DodgeTimerHandle, DodgeTimerDelegate, DodgeTime, false);
	}
}

bool UFlightLocomotionComponent::HandleCharacterLanding(const FHitResult& Hit)
{
	// Check if the hit surface is walkable
	if (CharacterMovement->IsWalkable(Hit))
	{
		float FallDistance = LandingInitiationLocationZ - OwnerCharacter->GetActorLocation().Z;

		// Check if the character fell from a significant height and divebomb was not initiated
		if (FallDistance > SoftLandingLimit && !bInitiatedDivebomb)
		{
			OwnerCharacter->DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));

			// Play hard landing montage if available
			if (ensure(HardLandingMontage != nullptr))
				OwnerCharacter->PlayAnimMontage(HardLandingMontage);

			return true;
		}
	}

	return false;
}

void UFlightLocomotionComponent::SetLandingInitiationLocationZ(float Value)
{
	LandingInitiationLocationZ = Value;
}

void UFlightLocomotionComponent::StopDivebomb()
{
	bInitiatedDivebomb = false;
	OwnerCharacter->StopAnimMontage(DivebombMontage);

	// Clear the divebomb timer
	GetWorld()->GetTimerManager().ClearTimer(DivebombTimerHandle);
}

void UFlightLocomotionComponent::UpdateFlightLocomotion(float DeltaTime)
{
	UpdateRotation(DeltaTime);

	UpdateBlendRates();

	if (bIsDodgingRight || bIsDodgingLeft)
	{
		ApplyDodgeForce(DeltaTime);
	}
}

void UFlightLocomotionComponent::UpdateRotation(float DeltaTime)
{
	FRotator CurrentRotation = CapsuleComponent->GetComponentRotation();

	FRotator TargetRotation;
	if (FlightLocomotionInterface->IsDashing())
	{
		TargetRotation = CharacterMovement->Velocity.Rotation();
	}
	else
	{
		TargetRotation = CameraComponent->GetComponentRotation();
		TargetRotation.Pitch = 0;
	}

	// Interpolate the current rotation towards the target rotation
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationInterpSpeed);
	OwnerCharacter->SetActorRotation(NewRotation);
}

void UFlightLocomotionComponent::UpdateBlendRates()
{
	FVector2D InputRange(-360.f, 360.f);
	FVector2D OutputRange(-1.f, 1.f);

	// Calculate the rotation rates based on the capsule's angular velocity
	FVector CapsuleAngularVelocity = CapsuleComponent->GetPhysicsAngularVelocityInDegrees();
	XRotationRate = FMath::GetMappedRangeValueClamped(InputRange, OutputRange, CapsuleAngularVelocity.Z);

	FTransform CharacterTransform = OwnerCharacter->GetTransform();
	FVector YLeanDirection = CharacterTransform.InverseTransformVector(CapsuleAngularVelocity);
	YRotationRate = FMath::GetMappedRangeValueClamped(InputRange, OutputRange, YLeanDirection.Y);
}

void UFlightLocomotionComponent::ApplyDodgeForce(float DeltaTime)
{
	// Calculate the dodge force based on the camera's right vector and the dodge direction
	FVector DodgeForce = CameraComponent->GetRightVector();
	if (bIsDodgingLeft)
	{
		DodgeForce *= -1.f;
	}
	DodgeForce *= CurrentDodgeForce;

	// Apply the dodge force to the character movement
	CharacterMovement->AddForce(DodgeForce);

	// Interpolate the dodge force towards 0 over time
	CurrentDodgeForce = FMath::FInterpTo(CurrentDodgeForce, 0.f, DeltaTime, DodgeInterpSpeed);
}

void UFlightLocomotionComponent::SmoothResetPitch(float DeltaTime)
{
	FRotator CapsuleRotation = CapsuleComponent->GetComponentRotation();

	// Check if the capsule's pitch is nearly zero
	if (FMath::IsNearlyZero(CapsuleRotation.Pitch))
	{
		// Set the actor's rotation to a target rotation with zero pitch
		FRotator TargetRotation = CapsuleComponent->GetComponentRotation();
		TargetRotation.Pitch = 0.f;

		OwnerCharacter->SetActorRotation(TargetRotation);
		bWasDashing = false;
	}
	else
	{
		// Interpolate the current rotation towards a target rotation with zero pitch
		FRotator CurrentRotation = CapsuleComponent->GetComponentRotation();
		FRotator TargetRotation = CapsuleComponent->GetComponentRotation();
		TargetRotation.Pitch = 0.f;

		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationInterpSpeed);
		OwnerCharacter->SetActorRotation(NewRotation);
	}
}

void UFlightLocomotionComponent::InitiateDivebombStart()
{
	float FallDistance = LandingInitiationLocationZ - OwnerCharacter->GetActorLocation().Z;

	// Check if the character has fallen from a distance greater than the dive engage height buffer
	if (FallDistance > DiveEngageHeightBuffer)
	{
		FHitResult Hit;
		FVector TraceStart = OwnerCharacter->GetActorLocation();
		FVector TraceEnd = TraceStart - FVector::UpVector * DiveEngageHeightBuffer * DiveEngageFloorCheckTraceRatio;

		// Perform a line trace to check for obstacles
		if (!GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, DivebombTraceParams))
		{
			bInitiatedDivebomb = true;

			// Play the divebomb montage if available
			if (ensure(DivebombMontage != nullptr))
				OwnerCharacter->PlayAnimMontage(DivebombMontage);

			// Set a timer for the divebomb start section length
			GetWorld()->GetTimerManager().SetTimer(DivebombTimerHandle, DivebombTimerDelegate, DivebombStartSectionLength, false);
		}
	}
}

void UFlightLocomotionComponent::UpdateDivebomb(float DeltaTime)
{
	FHitResult Hit;
	FVector TraceStart = OwnerCharacter->GetActorLocation();
	FVector TraceEnd = TraceStart - FVector::UpVector * CapsuleHalfHeight * DiveLandFloorCheckTraceRatio;

	// Set the character's velocity for divebombing
	CharacterMovement->Velocity = -FVector::UpVector * DivebombVelocity;

	// Perform a line trace to check for the landing surface
	if (!bIsLandingDivebomb && GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, DivebombTraceParams))
	{
		if (ensure(DivebombMontage != nullptr))
		{
			// Play the divebomb montage at a specified section and disable character input
			OwnerCharacter->PlayAnimMontage(DivebombMontage, 1.f, DiveMontageLandSectionName);
			OwnerCharacter->DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));

			// Execute the divebomb land event and set flags
			OnDivebombLandEnd.ExecuteIfBound(Hit.Location);

			bIsLandingDivebomb = true;

			// Set a timer for the divebomb land section length
			GetWorld()->GetTimerManager().SetTimer(DivebombLandTimerHandle, DivebombLandTimerDelegate, DivebombLandSectionLength, false);
		}
	}
}

void UFlightLocomotionComponent::ResetDodge()
{
	// Reset dodge flags and clear the dodge timer
	bIsDodgingRight = false;
	bIsDodgingLeft = false;

	GetWorld()->GetTimerManager().ClearTimer(DodgeTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(DodgeResetBufferTimerHandle, DodgeResetBufferTimerDelegate, DodgeBufferTime, false);
}

void UFlightLocomotionComponent::ResetDodgeTimer()
{
	// Reset the dodge flag and clear the dodge reset buffer timer
	bIsDodging = false;

	GetWorld()->GetTimerManager().ClearTimer(DodgeResetBufferTimerHandle);
}

void UFlightLocomotionComponent::InitiateDivebomb()
{
	bIsDivebombing = true;

	// Disable locomotion and execute the divebomb initiation event
	FlightLocomotionInterface->SetLocomotionEnabled(false);
	OnInitiateDivebomb.ExecuteIfBound();

	GetWorld()->GetTimerManager().ClearTimer(DivebombTimerHandle);
}

void UFlightLocomotionComponent::EndDivebombLand()
{
	// Reset divebombing and landing flags, clear timers, and reset values
	bIsDivebombing = false;
	bInitiatedDivebomb = false;
	bIsLandingDivebomb = false;

	LandingInitiationLocationZ = 0.f;
	CharacterMovement->Velocity = FVector::ZeroVector;
	FlightLocomotionInterface->SetLocomotionEnabled(true);

	GetWorld()->GetTimerManager().ClearTimer(DivebombLandTimerHandle);
}