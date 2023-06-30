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
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

void UFlightEffectsComponent::InitializeEffects()
{
	SonicBoomParticles = SetupParticleSystemComponent(SonicBoomEffect);
	DiveTrailParticles = SetupParticleSystemComponent(DiveTrailEffect);
	TakeoffChargeParticles = SetupParticleSystemComponent(TakeoffChargeEffect);

	HoverNiagara = SetupNiagaraComponent(HoverEffect);
	DashTrailNiagara = SetupNiagaraComponent(DashTrailEffect);

	WindAudio = SetupAudioComponent(WindSound);
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
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, LandEffect, LandLocation);
	UGameplayStatics::SpawnSoundAttached(DiveLandSound, OwnerCharacter->GetMesh());
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

	UGameplayStatics::SpawnSoundAttached(DiveLandSound, OwnerCharacter->GetMesh());
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

UParticleSystemComponent* UFlightEffectsComponent::SetupParticleSystemComponent(UParticleSystem* ParticleTemplate)
{
	UParticleSystemComponent* NewParticles = SetupAssociatedComponent<UParticleSystemComponent>();
	if (NewParticles != nullptr)
	{
		NewParticles->SetRelativeLocation(FVector(0.f, 0.f, 89.f));
		NewParticles->SetTemplate(ParticleTemplate);
	}

	return NewParticles;
}

UNiagaraComponent* UFlightEffectsComponent::SetupNiagaraComponent(UNiagaraSystem* NiagaraSystemAsset)
{
	UNiagaraComponent* NewNiagara = SetupAssociatedComponent<UNiagaraComponent>();
	if (NewNiagara != nullptr)
	{
		NewNiagara->SetRelativeLocation(FVector(0.f, 0.f, 89.f));
		NewNiagara->SetAsset(NiagaraSystemAsset);
	}

	return NewNiagara;
}

UAudioComponent* UFlightEffectsComponent::SetupAudioComponent(USoundBase* AudioSound)
{
	UAudioComponent* NewAudio = SetupAssociatedComponent<UAudioComponent>();
	if (NewAudio != nullptr)
	{
		NewAudio->SetRelativeLocation(FVector(0.f, 0.f, 89.f));
		NewAudio->SetSound(AudioSound);
	}

	return NewAudio;
}
