// Fill out your copyright notice in the Description page of Project Settings.


#include "Steelheart/Components/Public/FlightTakeoffComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
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


// Called when the game starts
void UFlightTakeoffComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UFlightTakeoffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsTakingOff)
	{
		CharacterMovement->AddForce(FVector::UpVector * BaseTakeOffForce);
	}
}

void UFlightTakeoffComponent::EngageTakeOff()
{
	if (OwnerCharacter->GetCharacterMovement()->IsMovingOnGround() && !bIsTakeOffInitiating)
	{
		bIsTakeOffInitiating = true;

		OwnerCharacter->PlayAnimMontage(TakeOffMontage);

		int32 EngageSectionIndex = TakeOffMontage->GetSectionIndex("Default");
		float EngageSectionLength = TakeOffMontage->GetSectionLength(EngageSectionIndex);

		GetWorld()->GetTimerManager().SetTimer(TakeOffLoopTimerHandle, TakeOffLoopTimerDelegate, EngageSectionLength, false);;
	}
}

void UFlightTakeoffComponent::ReleaseTakeOff()
{
	if (bIsTakeOffInitiating)
	{
		bIsTakeOffInitiating = false;

		if (bIsTakeOffCharged)
		{
			bIsTakeOffLooping = false;
			bIsTakeOffCharged = false;

			bIsTakingOff = true;
			OwnerCharacter->PlayAnimMontage(TakeOffMontage, 0.75f, ReleaseSectionName);
			CharacterMovement->SetMovementMode(MOVE_Flying);

			int32 ReleaseSectionIndex = TakeOffMontage->GetSectionIndex(LoopSectionName);
			float ReleaseSectionLength = TakeOffMontage->GetSectionLength(ReleaseSectionIndex);

			GetWorld()->GetTimerManager().SetTimer(TakeOffEndTimerHandle, TakeOffEndTimerDelegate, ReleaseSectionLength, false);;
		}
		else
		{
			float CurrentAnimPosition = OwnerCharacter->GetMesh()->GetPosition();

			OwnerCharacter->StopAnimMontage();
			OwnerCharacter->PlayAnimMontage(TakeOffMontage, -1.f);

			OwnerCharacter->GetMesh()->SetPosition(CurrentAnimPosition);
		}
	}
}

void UFlightTakeoffComponent::LoopTakeOff()
{
	if (bIsTakeOffLooping)
	{
		bIsTakeOffCharged = true;

		GetWorld()->GetTimerManager().ClearTimer(TakeOffLoopTimerHandle);
	}
	else
	{
		bIsTakeOffLooping = true;

		OwnerCharacter->PlayAnimMontage(TakeOffMontage, 1.f, LoopSectionName);

		int32 LoopSectionIndex = TakeOffMontage->GetSectionIndex(LoopSectionName);
		float LoopSectionLength = TakeOffMontage->GetSectionLength(LoopSectionIndex);

		GetWorld()->GetTimerManager().ClearTimer(TakeOffLoopTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(TakeOffLoopTimerHandle, TakeOffLoopTimerDelegate, LoopSectionLength, false);;
	}
}

void UFlightTakeoffComponent::EndTakeOff()
{
	bIsTakingOff = false;

	OwnerCharacter->StopAnimMontage();
	GetWorld()->GetTimerManager().ClearTimer(TakeOffEndTimerHandle);
}