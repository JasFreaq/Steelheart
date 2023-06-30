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

void UFlightEffectsComponent::InitializeFlightComponent()
{
	Super::InitializeFlightComponent();

	SonicBoomParticles = SetupParticleSystemComponent(SonicBoomEffect, SonicBoomDefaultPosition, SonicBoomDefaultOrientation);
	DiveTrailParticles = SetupParticleSystemComponent(DiveTrailEffect);
	TakeoffChargeParticles = SetupParticleSystemComponent(TakeoffChargeEffect);

	HoverNiagara = SetupNiagaraComponent(HoverEffect, HoverPosition);
	DashTrailNiagara = SetupNiagaraComponent(DashTrailEffect, FVector::Zero(), DashTrailOrientation);

	WindAudio = SetupAudioComponent(WindSound);
}

UParticleSystemComponent* UFlightEffectsComponent::SetupParticleSystemComponent(UParticleSystem* ParticleTemplate, FVector CompLoc, FRotator CompRot)
{
	UParticleSystemComponent* NewParticles = SetupAssociatedComponent<UParticleSystemComponent>(OwnerCharacter->GetMesh());
	if (NewParticles != nullptr)
	{
		NewParticles->SetRelativeLocationAndRotation(CompLoc, CompRot);
		NewParticles->SetTemplate(ParticleTemplate);
	}

	return NewParticles;
}

UNiagaraComponent* UFlightEffectsComponent::SetupNiagaraComponent(UNiagaraSystem* NiagaraSystemAsset, FVector CompLoc, FRotator CompRot)
{
	UNiagaraComponent* NewNiagara = SetupAssociatedComponent<UNiagaraComponent>(OwnerCharacter->GetMesh());
	if (NewNiagara != nullptr)
	{
		NewNiagara->SetRelativeLocationAndRotation(CompLoc, CompRot);
		NewNiagara->SetAsset(NiagaraSystemAsset);
	}

	return NewNiagara;
}

UAudioComponent* UFlightEffectsComponent::SetupAudioComponent(USoundBase* AudioSound, FVector CompLoc, FRotator CompRot)
{
	UAudioComponent* NewAudio = SetupAssociatedComponent<UAudioComponent>(OwnerCharacter->GetMesh());
	if (NewAudio != nullptr)
	{
		NewAudio->SetRelativeLocationAndRotation(CompLoc, CompRot);
		NewAudio->SetSound(AudioSound);
	}

	return NewAudio;
}
