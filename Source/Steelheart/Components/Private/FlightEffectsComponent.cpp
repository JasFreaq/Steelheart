// Fill out your copyright notice in the Description page of Project Settings.


#include "Steelheart/Components/Public/FlightEffectsComponent.h"

#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Steelheart/Components/Public/FlightEffectsComponent.h"

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
	UGameplayStatics::SpawnSoundAttached(SonicBoomSound, OwnerCharacter->GetMesh());
}

void UFlightEffectsComponent::ActivateHover()
{
	HoverNiagara->SetAsset(HoverEffect);
	HoverNiagara->Activate(true);
}

void UFlightEffectsComponent::ActivateDodge(bool Right)
{
	HoverNiagara->SetAsset(DodgeEffect);
	HoverNiagara->SetNiagaraVariableFloat(FString(TEXT("User.Direction")), Right ? 1.f : -1.f);
	HoverNiagara->Activate(true);
}

void UFlightEffectsComponent::ActivateDiveTrail()
{
	SonicBoomParticles->SetRelativeRotation(SonicBoomDiveOrientation);
	SonicBoomParticles->Activate(true);
	SonicBoomAudio = UGameplayStatics::SpawnSoundAttached(SonicBoomSound, OwnerCharacter->GetMesh());

	DiveTrailParticles->Activate(true);
}

void UFlightEffectsComponent::ActivateHardLanding(FVector LandLocation)
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, HardLandingEffect, LandLocation);
	UGameplayStatics::SpawnSoundAttached(LandSound, OwnerCharacter->GetMesh());
}

void UFlightEffectsComponent::ActivateDiveLand(FVector LandLocation)
{
	DiveTrailParticles->Deactivate();

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, DiveLandEffect, LandLocation);
	
	if (SonicBoomAudio != nullptr)
	{
		SonicBoomAudio->Stop();
		SonicBoomAudio = nullptr;
	}
	UGameplayStatics::SpawnSoundAttached(LandSound, OwnerCharacter->GetMesh());
}

void UFlightEffectsComponent::ToggleDashTrail(bool Enable)
{
	if (Enable)
	{
		DashTrailNiagara->Activate(true);
		WindAudio->Activate(true);
	}
	else
	{
		DashTrailNiagara->Deactivate();
		WindAudio->Deactivate();
	}
}

void UFlightEffectsComponent::ToggleTakeOffCharge(bool Enable, bool Activate)
{
	if (Enable)
	{
		TakeoffChargeParticles->Activate(true);
	}
	else
	{
		TakeoffChargeParticles->Deactivate();

		if (Activate)
		{
			SonicBoomParticles->SetRelativeRotation(SonicBoomTakeoffOrientation);
			SonicBoomParticles->Activate(true);
			UGameplayStatics::SpawnSoundAttached(SonicBoomSound, OwnerCharacter->GetMesh());
		}
	}
}