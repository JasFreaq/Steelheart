// Fill out your copyright notice in the Description page of Project Settings.


#include "Steelheart/Components/Public/FlightLocomotionComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UFlightLocomotionComponent::UFlightLocomotionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	DodgeTimerDelegate.BindUFunction(this, "ResetDodge");
	DodgeResetBufferTimerDelegate.BindUFunction(this, "ResetDodgeTimer");
}

void UFlightLocomotionComponent::InitializeFlightLocomotion(ACharacter* OwnerCharacterRef,
	UCameraComponent* CameraComponentRef, UCapsuleComponent* CapsuleComponentRef,
	UCharacterMovementComponent* CharacterMovementRef)
{
	OwnerCharacter = OwnerCharacterRef;
	CameraComponent = CameraComponentRef;
	CapsuleComponent = CapsuleComponentRef;
	CharacterMovement = CharacterMovementRef;

	CapsuleHalfHeight = CapsuleComponent->GetUnscaledCapsuleHalfHeight();

	CharacterMovement->MaxAcceleration = BaseAcceleration;
	CharacterMovement->BrakingDecelerationFlying = BrakingDecelerationFlying;
}


// Called every frame
void UFlightLocomotionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CharacterMovement->IsFlying())
	{
		UpdateFlightLocomotion(DeltaTime);
	}

	/*if (bWasDashing)
	{
		SmoothResetPitch(DeltaTime);
	}*/
}


void UFlightLocomotionComponent::UpdateFlightLocomotion(float DeltaTime)
{
	UpdateFlightRotation(DeltaTime);

	UpdateBlendRates();

	if (bIsDodgingRight || bIsDodgingLeft)
	{
		ApplyDodgeForce(DeltaTime);
	}
}

void UFlightLocomotionComponent::Fly()
{
	CharacterMovement->MaxFlySpeed = BaseSpeed;
	CharacterMovement->MaxAcceleration = BaseAcceleration;

	CharacterMovement->SetMovementMode(MOVE_Flying);
}

void UFlightLocomotionComponent::StopFlying()
{
	FVector CapsuleLinearVelocity = CapsuleComponent->GetPhysicsLinearVelocity();
	float ZMomentum = CapsuleLinearVelocity.Z;

	CharacterMovement->SetMovementMode(MOVE_Falling);

	FVector LaunchVelocity(0.f, 0.f, ZMomentum * ZMomentumCoeff);
	OwnerCharacter->LaunchCharacter(LaunchVelocity, false, true);
}

void UFlightLocomotionComponent::Dash()
{
	bIsDashing = true;

	CharacterMovement->MaxFlySpeed = DashSpeed;
	CharacterMovement->MaxAcceleration = DashAcceleration;

	CapsuleComponent->SetCapsuleHalfHeight(CapsuleComponent->GetUnscaledCapsuleRadius());
		
	bWasDashing = false;
}

void UFlightLocomotionComponent::StopDashing()
{
	bIsDashing = false;

	CharacterMovement->MaxFlySpeed = BaseSpeed;
	CharacterMovement->MaxAcceleration = BaseAcceleration;

	CapsuleComponent->SetCapsuleHalfHeight(CapsuleHalfHeight);

	bIsDodgingRight = false;
	bIsDodgingLeft = false;

	bWasDashing = true;
}

void UFlightLocomotionComponent::RightDodge()
{
	if (bIsDashing && !bIsDodging && !(bIsDodgingRight || bIsDodgingLeft))
	{
		bIsDodging = true;
		bIsDodgingRight = true;

		CurrentDodgeForce = BaseDodgeForce;
		
		GetWorld()->GetTimerManager().SetTimer(DodgeTimerHandle, DodgeTimerDelegate, DodgeTime, false);
	}
}

void UFlightLocomotionComponent::LeftDodge()
{
	if (bIsDashing && !bIsDodging && !(bIsDodgingRight || bIsDodgingLeft))
	{
		bIsDodging = true;
		bIsDodgingLeft = true;

		CurrentDodgeForce = BaseDodgeForce;

		GetWorld()->GetTimerManager().SetTimer(DodgeTimerHandle, DodgeTimerDelegate, DodgeTime, false);
	}
}

void UFlightLocomotionComponent::UpdateFlightRotation(float DeltaTime)
{
	FRotator CurrentRotation = CapsuleComponent->GetComponentRotation();

	FRotator TargetRotation = CameraComponent->GetComponentRotation();
	if (!bIsDashing)
	{
		TargetRotation.Pitch = 0.f;
	}
	
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationInterpSpeed);
	OwnerCharacter->SetActorRotation(NewRotation);
}

void UFlightLocomotionComponent::UpdateBlendRates()
{
	FVector2D InputRange(-360.f, 360.f);
	FVector2D OutputRange(-1.f, 1.f);

	FVector CapsuleAngularVelocity = CapsuleComponent->GetPhysicsAngularVelocityInDegrees();
	XRotationRate = FMath::GetMappedRangeValueClamped(InputRange, OutputRange, CapsuleAngularVelocity.Z);

	FTransform CharacterTransform = OwnerCharacter->GetTransform();
	FVector YLeanDirection = CharacterTransform.InverseTransformVector(CapsuleAngularVelocity);
	YRotationRate = FMath::GetMappedRangeValueClamped(InputRange, OutputRange, YLeanDirection.Y);
}

void UFlightLocomotionComponent::ApplyDodgeForce(float DeltaTime)
{
	FVector DodgeForce = CameraComponent->GetRightVector();
	if (bIsDodgingLeft)
	{
		DodgeForce *= -1.f;
	}
	DodgeForce *= CurrentDodgeForce;

	CharacterMovement->AddForce(DodgeForce);

	CurrentDodgeForce = FMath::FInterpTo(CurrentDodgeForce, 0.f, DeltaTime, DodgeInterpSpeed);
}

void UFlightLocomotionComponent::SmoothResetPitch(float DeltaTime)
{
	FRotator CapsuleRotation = CapsuleComponent->GetComponentRotation();
	if (FMath::IsNearlyZero(CapsuleRotation.Pitch))
	{
		FRotator TargetRotation = CapsuleComponent->GetComponentRotation();
		TargetRotation.Pitch = 0.f;

		OwnerCharacter->SetActorRotation(TargetRotation);
		bWasDashing = false;
	}
	else
	{
		FRotator CurrentRotation = CapsuleComponent->GetComponentRotation();
		FRotator TargetRotation = CapsuleComponent->GetComponentRotation();
		TargetRotation.Pitch = 0.f;

		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationInterpSpeed);
		OwnerCharacter->SetActorRotation(NewRotation);
	}
}

void UFlightLocomotionComponent::ResetDodge()
{
	bIsDodgingRight = false;
	bIsDodgingLeft = false;

	GetWorld()->GetTimerManager().ClearTimer(DodgeTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(DodgeResetBufferTimerHandle, DodgeResetBufferTimerDelegate, DodgeBufferTime, false);
}

void UFlightLocomotionComponent::ResetDodgeTimer()
{
	bIsDodging = false;

	GetWorld()->GetTimerManager().ClearTimer(DodgeResetBufferTimerHandle);
}
