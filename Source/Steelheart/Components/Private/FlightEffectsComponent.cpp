// Fill out your copyright notice in the Description page of Project Settings.

#include "Steelheart/Components/Public/FlightEffectsComponent.h"

#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values for this component's properties
UFlightEffectsComponent::UFlightEffectsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.
	// You can turn these features off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

void UFlightEffectsComponent::ActivateSonicBoom()
{
	// Set the relative rotation of SonicBoomParticles
	SonicBoomParticles->SetRelativeRotation(SonicBoomDefaultOrientation);

	// Activate SonicBoomParticles and play the associated sound
	SonicBoomParticles->Activate(true);
	UGameplayStatics::SpawnSoundAttached(SonicBoomSound, OwnerCharacter->GetMesh());
}

void UFlightEffectsComponent::ActivateHover()
{
	// Set the Niagara asset for HoverNiagara and activate it
	HoverNiagara->SetAsset(HoverEffect);
	HoverNiagara->Activate(true);
}

void UFlightEffectsComponent::ActivateDodge(bool Right)
{
	// Set the Niagara asset for HoverNiagara and set the direction parameter based on 'Right' flag
	HoverNiagara->SetAsset(DodgeEffect);
	HoverNiagara->SetNiagaraVariableFloat(FString(TEXT("User.Direction")), Right ? 1.f : -1.f);
	HoverNiagara->Activate(true);
}

void UFlightEffectsComponent::ActivateDiveTrail()
{
	// Set the relative rotation of SonicBoomParticles for diving
	SonicBoomParticles->SetRelativeRotation(SonicBoomDiveOrientation);

	// Activate SonicBoomParticles and play the associated sound
	SonicBoomParticles->Activate(true);
	SonicBoomAudio = UGameplayStatics::SpawnSoundAttached(SonicBoomSound, OwnerCharacter->GetMesh());

	// Activate DiveTrailParticles
	DiveTrailParticles->Activate(true);
}

void UFlightEffectsComponent::ActivateHardLanding(FVector LandLocation)
{
	// Spawn LandEffect Niagara system at the specified location
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, LandEffect, LandLocation);

	// Play the DiveLandSound attached to the OwnerCharacter's mesh
	UGameplayStatics::SpawnSoundAttached(DiveLandSound, OwnerCharacter->GetMesh());
}

void UFlightEffectsComponent::ActivateDiveLand(FVector LandLocation)
{
	// Deactivate DiveTrailParticles
	DiveTrailParticles->Deactivate();

	// Spawn DiveLandEffect Niagara system at the specified location
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, DiveLandEffect, LandLocation);

	// Stop and reset SonicBoomAudio if it exists
	if (SonicBoomAudio != nullptr)
	{
		SonicBoomAudio->Stop();
		SonicBoomAudio = nullptr;
	}

	// Play the DiveLandSound attached to the OwnerCharacter's mesh
	UGameplayStatics::SpawnSoundAttached(DiveLandSound, OwnerCharacter->GetMesh());
}

void UFlightEffectsComponent::ToggleDashTrail(bool Enable)
{
	if (Enable)
	{
		// Activate DashTrailNiagara and WindAudio
		DashTrailNiagara->Activate(true);
		WindAudio->Activate(true);
	}
	else
	{
		// Deactivate DashTrailNiagara and WindAudio
		DashTrailNiagara->Deactivate();
		WindAudio->Deactivate();
	}
}

void UFlightEffectsComponent::ToggleTakeOffCharge(bool Enable, bool Activate)
{
	if (Enable)
	{
		// Activate TakeoffChargeParticles
		TakeoffChargeParticles->Activate(true);
	}
	else
	{
		// Deactivate TakeoffChargeParticles

		TakeoffChargeParticles->Deactivate();

		if (Activate)
		{
			// Set the relative rotation of SonicBoomParticles for takeoff
			SonicBoomParticles->SetRelativeRotation(SonicBoomTakeoffOrientation);

			// Activate SonicBoomParticles and play the associated sound
			SonicBoomParticles->Activate(true);
			UGameplayStatics::SpawnSoundAttached(SonicBoomSound, OwnerCharacter->GetMesh());
		}
	}
}

void UFlightEffectsComponent::InitializeFlightComponent()
{
	Super::InitializeFlightComponent();

	// Set up SonicBoomParticles with the specified particle system, position, and rotation
	SonicBoomParticles = SetupParticleSystemComponent(SonicBoomEffect, SonicBoomDefaultPosition, SonicBoomDefaultOrientation);

	// Set up DiveTrailParticles with the specified particle system
	DiveTrailParticles = SetupParticleSystemComponent(DiveTrailEffect);

	// Set up TakeoffChargeParticles with the specified particle system
	TakeoffChargeParticles = SetupParticleSystemComponent(TakeoffChargeEffect);

	// Set up HoverNiagara with the specified Niagara system and position
	HoverNiagara = SetupNiagaraComponent(HoverEffect, HoverPosition);

	// Set up DashTrailNiagara with the specified Niagara system and orientation
	DashTrailNiagara = SetupNiagaraComponent(DashTrailEffect, FVector::Zero(), DashTrailOrientation);

	// Set up WindAudio component with the specified sound
	WindAudio = SetupAudioComponent(WindSound);
}

UParticleSystemComponent* UFlightEffectsComponent::SetupParticleSystemComponent(UParticleSystem* ParticleTemplate, FVector CompLoc, FRotator CompRot)
{
	// Set up and configure a new particle system component
	UParticleSystemComponent* NewParticles = SetupAssociatedComponent<UParticleSystemComponent>(OwnerCharacter->GetMesh());
	if (NewParticles != nullptr)
	{
		// Set the relative location and rotation of the particle system component
		NewParticles->SetRelativeLocationAndRotation(CompLoc, CompRot);

		// Set the particle template for the particle system component
		NewParticles->SetTemplate(ParticleTemplate);
	}

	return NewParticles;
}

UNiagaraComponent* UFlightEffectsComponent::SetupNiagaraComponent(UNiagaraSystem* NiagaraSystemAsset, FVector CompLoc, FRotator CompRot)
{
	// Set up and configure a new Niagara component
	UNiagaraComponent* NewNiagara = SetupAssociatedComponent<UNiagaraComponent>(OwnerCharacter->GetMesh());
	if (NewNiagara != nullptr)
	{
		// Set the relative location and rotation of the Niagara component
		NewNiagara->SetRelativeLocationAndRotation(CompLoc, CompRot);

		// Set the Niagara asset for the Niagara component
		NewNiagara->SetAsset(NiagaraSystemAsset);
	}

	return NewNiagara;
}

UAudioComponent* UFlightEffectsComponent::SetupAudioComponent(USoundBase* AudioSound, FVector CompLoc, FRotator CompRot)
{
	// Set up and configure a new audio component
	UAudioComponent* NewAudio = SetupAssociatedComponent<UAudioComponent>(OwnerCharacter->GetMesh());
	if (NewAudio != nullptr)
	{
		// Set the relative location and rotation of the audio component
		NewAudio->SetRelativeLocationAndRotation(CompLoc, CompRot);

		// Set the sound for the audio component
		NewAudio->SetSound(AudioSound);
	}

	return NewAudio;
}
