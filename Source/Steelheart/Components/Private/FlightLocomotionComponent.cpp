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
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

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

	CapsuleHalfHeight = CapsuleComponent->GetUnscaledCapsuleHalfHeight();

	CharacterMovement->MaxAcceleration = BaseAcceleration;
	CharacterMovement->BrakingDecelerationFlying = BrakingDecelerationFlying;

	if (ensure(DivebombMontage != nullptr))
	{
		int32 StartSectionIndex = DivebombMontage->GetSectionIndex("Default");
		DivebombStartSectionLength = DivebombMontage->GetSectionLength(StartSectionIndex) / DIVEBOMB_RATE_SCALE;

		int32 LandSectionIndex = DivebombMontage->GetSectionIndex(DiveMontageLandSectionName);
		DivebombLandSectionLength = DivebombMontage->GetSectionLength(LandSectionIndex) / DIVEBOMB_RATE_SCALE;
	}

	DivebombTraceParams.AddIgnoredActor(OwnerCharacter);
	DivebombTraceParams.bTraceComplex = true;
}

// Called every frame
void UFlightLocomotionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CharacterMovement->IsFlying())
	{
		UpdateFlightLocomotion(DeltaTime);
	}
	else if (CharacterMovement->IsFalling())
	{
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
	
	if (bWasDashing)
	{
		SmoothResetPitch(DeltaTime);
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
	LandingInitiationLocationZ = OwnerCharacter->GetActorLocation().Z;

	FVector CapsuleLinearVelocity = CapsuleComponent->GetPhysicsLinearVelocity();
	float ZMomentum = CapsuleLinearVelocity.Z;
	
	FVector LaunchVelocity(0.f, 0.f, ZMomentum * ZMomentumCoeff);
	OwnerCharacter->LaunchCharacter(LaunchVelocity, false, true);

	CharacterMovement->SetMovementMode(MOVE_Falling);
	CapsuleComponent->SetCapsuleHalfHeight(CapsuleHalfHeight);
}

void UFlightLocomotionComponent::Dash()
{
	CharacterMovement->MaxFlySpeed = DashSpeed;
	CharacterMovement->MaxAcceleration = DashAcceleration;

	CapsuleComponent->SetCapsuleHalfHeight(CapsuleComponent->GetUnscaledCapsuleRadius());
		
	bWasDashing = false;
}

void UFlightLocomotionComponent::StopDashing()
{
	CharacterMovement->MaxFlySpeed = BaseSpeed;
	CharacterMovement->MaxAcceleration = BaseAcceleration;

	CapsuleComponent->SetCapsuleHalfHeight(CapsuleHalfHeight);

	bIsDodgingRight = false;
	bIsDodgingLeft = false;

	bWasDashing = true;
}

void UFlightLocomotionComponent::RightDodge()
{
	if (FlightLocomotionInterface->IsDashing() && !bIsDodging && !(bIsDodgingRight || bIsDodgingLeft))
	{
		bIsDodging = true;
		bIsDodgingRight = true;

		CurrentDodgeForce = BaseDodgeForce;
		
		GetWorld()->GetTimerManager().SetTimer(DodgeTimerHandle, DodgeTimerDelegate, DodgeTime, false);
	}
}

void UFlightLocomotionComponent::LeftDodge()
{
	if (FlightLocomotionInterface->IsDashing() && !bIsDodging && !(bIsDodgingRight || bIsDodgingLeft))
	{
		bIsDodging = true;
		bIsDodgingLeft = true;

		CurrentDodgeForce = BaseDodgeForce;

		GetWorld()->GetTimerManager().SetTimer(DodgeTimerHandle, DodgeTimerDelegate, DodgeTime, false);
	}
}

bool UFlightLocomotionComponent::HandleCharacterLanding(const FHitResult& Hit)
{
	if (CharacterMovement->IsWalkable(Hit))
	{
		float FallDistance = LandingInitiationLocationZ - OwnerCharacter->GetActorLocation().Z;

		if (FallDistance > SoftLandingLimit && !bInitiatedDivebomb)
		{
			OwnerCharacter->DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));

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

void UFlightLocomotionComponent::InitiateDivebombStart()
{
	float FallDistance = LandingInitiationLocationZ - OwnerCharacter->GetActorLocation().Z;
	if (FallDistance > DiveEngageHeightBuffer)
	{
		FHitResult Hit;
		FVector TraceStart = OwnerCharacter->GetActorLocation();
		FVector TraceEnd = TraceStart - FVector::UpVector * DiveEngageHeightBuffer * DiveEngageFloorCheckTraceRatio;

		if (!GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, DivebombTraceParams))
		{
			bInitiatedDivebomb = true;

			if (ensure(DivebombMontage != nullptr))
				OwnerCharacter->PlayAnimMontage(DivebombMontage);
			
			GetWorld()->GetTimerManager().SetTimer(DivebombTimerHandle, DivebombTimerDelegate, DivebombStartSectionLength, false);
		}
	}
}

void UFlightLocomotionComponent::UpdateDivebomb(float DeltaTime)
{
	FHitResult Hit;
	FVector TraceStart = OwnerCharacter->GetActorLocation();
	FVector TraceEnd = TraceStart - FVector::UpVector * CapsuleHalfHeight * DiveLandFloorCheckTraceRatio;

	CharacterMovement->Velocity = -FVector::UpVector * DivebombVelocity;

	if (!bIsLandingDivebomb && GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, DivebombTraceParams))
	{			
		if (ensure(DivebombMontage != nullptr))
		{
			OwnerCharacter->PlayAnimMontage(DivebombMontage, 1.f, DiveMontageLandSectionName);
			OwnerCharacter->DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));

			OnDivebombLandEnd.ExecuteIfBound(Hit.Location);

			bIsLandingDivebomb = true;

			GetWorld()->GetTimerManager().SetTimer(DivebombLandTimerHandle, DivebombLandTimerDelegate, DivebombLandSectionLength, false);
		}
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

void UFlightLocomotionComponent::InitiateDivebomb()
{
	bIsDivebombing = true;

	FlightLocomotionInterface->SetLocomotionEnabled(false);
	OnInitiateDivebomb.ExecuteIfBound();

	GetWorld()->GetTimerManager().ClearTimer(DivebombTimerHandle);
}

void UFlightLocomotionComponent::EndDivebombLand()
{
	bIsDivebombing = false;
	bInitiatedDivebomb = false;
	bIsLandingDivebomb = false;

	LandingInitiationLocationZ = 0.f;
	CharacterMovement->Velocity = FVector::ZeroVector;
	FlightLocomotionInterface->SetLocomotionEnabled(true);
	
	GetWorld()->GetTimerManager().ClearTimer(DivebombLandTimerHandle);
}
