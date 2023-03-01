// Fill out your copyright notice in the Description page of Project Settings.


#include "Steelheart/Components/Public/FlightTakeoffComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Steelheart/Interfaces/Public/FlightLocomotionInterface.h"

// Sets default values for this component's properties
UFlightTakeoffComponent::UFlightTakeoffComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	TakeOffLoopTimerDelegate.BindUFunction(this, "LoopTakeOff");
	TakeOffEndTimerDelegate.BindUFunction(this, "EndTakeOff");
}

void UFlightTakeoffComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ensure(TakeOffMontage != nullptr))
	{
		BaseBlendOutTime = TakeOffMontage->BlendOut.GetBlendTime();

		int32 EngageSectionIndex = TakeOffMontage->GetSectionIndex("Default");
		EngageSectionLength = TakeOffMontage->GetSectionLength(EngageSectionIndex);

		int32 LoopSectionIndex = TakeOffMontage->GetSectionIndex(LoopSectionName);
		LoopSectionLength = TakeOffMontage->GetSectionLength(LoopSectionIndex);

		int32 ReleaseSectionIndex = TakeOffMontage->GetSectionIndex(ReleaseSectionName);
		ReleaseSectionLength = TakeOffMontage->GetSectionLength(ReleaseSectionIndex);
	}
}

// Called every frame
void UFlightTakeoffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsTakingOff)
	{
		CharacterMovement->AddForce(FVector::UpVector * CurrentTakeOffForce);

		ReleaseTimeCounter += DeltaTime;
		float ReleaseForceAlpha = ReleaseTimeCounter / ReleaseSectionLength;
		CurrentTakeOffForce = FMath::Lerp(BaseTakeOffForce, 0.f, ReleaseForceAlpha);
	}
}

void UFlightTakeoffComponent::EngageTakeOff()
{
	if (ensure(TakeOffMontage != nullptr) &&
		OwnerCharacter->GetCharacterMovement()->IsMovingOnGround() && !bIsTakeOffInitiating)
	{
		bIsTakeOffInitiating = true;
		FlightLocomotionInterface->SetLocomotionEnabled(false);

		TakeOffMontage->BlendOut.SetBlendTime(BaseBlendOutTime);
		OwnerCharacter->PlayAnimMontage(TakeOffMontage);
				
		GetWorld()->GetTimerManager().SetTimer(TakeOffLoopTimerHandle, TakeOffLoopTimerDelegate, EngageSectionLength, false);;
	}
}

void UFlightTakeoffComponent::ReleaseTakeOff()
{
	if (ensure(TakeOffMontage != nullptr) && bIsTakeOffInitiating)
	{
		bIsTakeOffInitiating = false;
		float SectionLength;

		if (bIsTakeOffCharged)
		{
			bIsTakeOffLooping = false;
			bIsTakeOffCharged = false;
			bIsTakingOff = true;

			OwnerCharacter->PlayAnimMontage(TakeOffMontage, RATE_SCALE, ReleaseSectionName);
			OnReleaseTakeoff.ExecuteIfBound(true);

			ReleaseTimeCounter = 0.f;
			CurrentTakeOffForce = BaseTakeOffForce;

			SectionLength = ReleaseSectionLength;
		}
		else 
		{
			TakeOffMontage->BlendOut.SetBlendTime(EngageSectionLength / 2);
			OwnerCharacter->StopAnimMontage(TakeOffMontage);
			OnReleaseTakeoff.ExecuteIfBound(false);

			SectionLength = EngageSectionLength / 2;
			GetWorld()->GetTimerManager().ClearTimer(TakeOffLoopTimerHandle);
		}

		GetWorld()->GetTimerManager().SetTimer(TakeOffEndTimerHandle, TakeOffEndTimerDelegate, SectionLength, false);;
	}
}

void UFlightTakeoffComponent::LoopTakeOff()
{
	if (ensure(TakeOffMontage != nullptr))
	{
		if (bIsTakeOffLooping)
		{
			bIsTakeOffCharged = true;

			GetWorld()->GetTimerManager().ClearTimer(TakeOffLoopTimerHandle);
		}
		else
		{
			bIsTakeOffLooping = true;

			OwnerCharacter->PlayAnimMontage(TakeOffMontage, RATE_SCALE, LoopSectionName);
		
			GetWorld()->GetTimerManager().ClearTimer(TakeOffLoopTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(TakeOffLoopTimerHandle, TakeOffLoopTimerDelegate, LoopSectionLength, false);;
		}
	}
}

void UFlightTakeoffComponent::EndTakeOff()
{
	if (ensure(TakeOffMontage != nullptr))
	{
		bIsTakingOff = false;
		FlightLocomotionInterface->SetLocomotionEnabled(true);

		OwnerCharacter->StopAnimMontage();
		GetWorld()->GetTimerManager().ClearTimer(TakeOffEndTimerHandle);
	}
}