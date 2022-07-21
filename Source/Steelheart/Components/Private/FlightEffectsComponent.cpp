// Fill out your copyright notice in the Description page of Project Settings.


#include "Steelheart/Components/Public/FlightEffectsComponent.h"

#include "GameFramework/Character.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values for this component's properties
UFlightEffectsComponent::UFlightEffectsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

void UFlightEffectsComponent::ActivateSonicBoom()
{
	SonicBoomParticles->SetRelativeRotation(SonicBoomDefaultOrientation);
	
	SonicBoomParticles->Activate(true);
}

void UFlightEffectsComponent::ActivateDiveTrail()
{
	SonicBoomParticles->SetRelativeRotation(SonicBoomDiveOrientation);
	SonicBoomParticles->Activate(true);

	DiveTrailParticles->Activate(true);
}

void UFlightEffectsComponent::ActivateDiveLand(FVector LandLocation)
{
	DiveTrailParticles->Deactivate();

	UGameplayStatics::SpawnEmitterAtLocation(this, DiveLandEffect, LandLocation);
}

void UFlightEffectsComponent::ToggleDashTrail(bool Enable)
{
	if (Enable)
	{
		DashTrailNiagara->Activate(true);
	}
	else
	{
		DashTrailNiagara->Deactivate();
	}
}
