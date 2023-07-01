// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlightComponent.h"
#include "FlightEffectsComponent.generated.h"

// Forward declarations
class UParticleSystem;
class UParticleSystemComponent;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS(ClassGroup = (FlightLocomotion))
class STEELHEART_API UFlightEffectsComponent : public UFlightComponent
{
	GENERATED_BODY()

	// Particle system components for various effects
	UParticleSystemComponent* SonicBoomParticles;

	UParticleSystemComponent* DiveTrailParticles;

	UParticleSystemComponent* TakeoffChargeParticles;

	// Niagara components for hover and dash trail effects
	UNiagaraComponent* HoverNiagara;

	UNiagaraComponent* DashTrailNiagara;

	// Audio components for wind and sonic boom sounds
	UAudioComponent* WindAudio;

	UAudioComponent* SonicBoomAudio;

public:
	// Sets default values for this component's properties
	UFlightEffectsComponent();

	// Functions to activate various flight effects
	void ActivateSonicBoom();

	void ActivateHover();

	void ActivateDodge(bool Right);

	void ActivateDiveTrail();

	void ActivateHardLanding(FVector LandLocation);

	void ActivateDiveLand(FVector LandLocation);

	void ToggleDashTrail(bool Enable);

	void ToggleTakeOffCharge(bool Enable, bool Activate = false);

protected:
	virtual void InitializeFlightComponent() override;

private:
	// Miscellaneous Flight

	UPROPERTY(EditDefaultsOnly, Category = MiscFlightEffect)
		UParticleSystem* TakeoffChargeEffect;

	UPROPERTY(EditDefaultsOnly, Category = MiscFlightEffect)
		UNiagaraSystem* HoverEffect;

	UPROPERTY(EditDefaultsOnly, Category = MiscFlightEffect)
		FVector HoverPosition = FVector(0, 0, 89);

	UPROPERTY(EditDefaultsOnly, Category = MiscFlightEffect)
		UNiagaraSystem* DashTrailEffect;

	UPROPERTY(EditDefaultsOnly, Category = MiscFlightEffect)
		FRotator DashTrailOrientation = FRotator(0, 90, 0);

	UPROPERTY(EditDefaultsOnly, Category = MiscFlightEffect)
		UNiagaraSystem* DodgeEffect;

	UPROPERTY(EditDefaultsOnly, Category = MiscFlightEffect)
		UNiagaraSystem* LandEffect;

	UPROPERTY(EditDefaultsOnly, Category = MiscFlightEffect)
		USoundBase* WindSound;

	// Sonic Boom

	UPROPERTY(EditDefaultsOnly, Category = SonicBoomEffect)
		UParticleSystem* SonicBoomEffect;

	UPROPERTY(EditDefaultsOnly, Category = SonicBoomEffect)
		USoundBase* SonicBoomSound;

	UPROPERTY(EditDefaultsOnly, Category = SonicBoomEffect)
		FVector SonicBoomDefaultPosition = FVector(0, 0, 89);

	UPROPERTY(EditDefaultsOnly, Category = SonicBoomEffect)
		FVector SonicBoomDivePosition = FVector(0, 0, 178);

	UPROPERTY(EditDefaultsOnly, Category = SonicBoomEffect)
		FRotator SonicBoomDefaultOrientation = FRotator(0, -90, 0);

	UPROPERTY(EditDefaultsOnly, Category = SonicBoomEffect)
		FRotator SonicBoomDiveOrientation = FRotator(90, 0, 0);;

	UPROPERTY(EditDefaultsOnly, Category = SonicBoomEffect)
		FRotator SonicBoomTakeoffOrientation = FRotator(-90, 0, 0);;

	UPROPERTY(EditDefaultsOnly, Category = SonicBoomEffect)
		float SonicBoomSoundStartTime = 1.f;

	// Dive

	UPROPERTY(EditDefaultsOnly, Category = DiveEffect)
		UParticleSystem* DiveTrailEffect;

	UPROPERTY(EditDefaultsOnly, Category = DiveEffect)
		UNiagaraSystem* DiveLandEffect;

	UPROPERTY(EditDefaultsOnly, Category = DiveEffect)
		USoundBase* DiveLandSound;

	// Helper functions for setting up particle system, Niagara, and audio components
	UParticleSystemComponent* SetupParticleSystemComponent(UParticleSystem* ParticleTemplate, FVector CompLoc = FVector(0, 0, 0), FRotator CompRot = FRotator(0, 0, 0));

	UNiagaraComponent* SetupNiagaraComponent(UNiagaraSystem* NiagaraSystemAsset, FVector CompLoc = FVector(0, 0, 0), FRotator CompRot = FRotator(0, 0, 0));

	UAudioComponent* SetupAudioComponent(USoundBase* AudioSound, FVector CompLoc = FVector(0, 0, 0), FRotator CompRot = FRotator(0, 0, 0));
};
