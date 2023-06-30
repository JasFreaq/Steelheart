// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlightComponent.h"
#include "FlightEffectsComponent.generated.h"

class UParticleSystem;
class UParticleSystemComponent;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS(ClassGroup = (FlightLocomotion))
class STEELHEART_API UFlightEffectsComponent : public UFlightComponent
{
	GENERATED_BODY()

	UParticleSystemComponent* SonicBoomParticles;

	UParticleSystemComponent* DiveTrailParticles;

	UParticleSystemComponent* TakeoffChargeParticles;
	
	UNiagaraComponent* HoverNiagara;

	UNiagaraComponent* DashTrailNiagara;

	UAudioComponent* WindAudio;

	UAudioComponent* SonicBoomAudio;

public:	
	// Sets default values for this component's properties
	UFlightEffectsComponent();
	
	void InitializeEffects();

	void ActivateSonicBoom();

	void ActivateHover();

	void ActivateDodge(bool Right);

	void ActivateDiveTrail();

	void ActivateHardLanding(FVector LandLocation);

	void ActivateDiveLand(FVector LandLocation);

	void ToggleDashTrail(bool Enable);

	void ToggleTakeOffCharge(bool Enable, bool Activate = false);

private:
	//Misc Flight

	UPROPERTY(EditDefaultsOnly, Category = MiscFlightEffect)
		UParticleSystem* TakeoffChargeEffect;

	UPROPERTY(EditDefaultsOnly, Category = MiscFlightEffect)
		UNiagaraSystem* HoverEffect;

	UPROPERTY(EditDefaultsOnly, Category = MiscFlightEffect)
		UNiagaraSystem* DashTrailEffect;

	UPROPERTY(EditDefaultsOnly, Category = MiscFlightEffect)
		UNiagaraSystem* DodgeEffect;

	UPROPERTY(EditDefaultsOnly, Category = MiscFlightEffect)
		UNiagaraSystem* LandEffect;

	UPROPERTY(EditDefaultsOnly, Category = MiscFlightEffect)
		USoundBase* WindSound;

	//Sonic Boom

	UPROPERTY(EditDefaultsOnly, Category = SonicBoomEffect)
		UParticleSystem* SonicBoomEffect;

	UPROPERTY(EditDefaultsOnly, Category = SonicBoomEffect)
		USoundBase* SonicBoomSound;

	UPROPERTY(EditDefaultsOnly, Category = SonicBoomEffect)
		FRotator SonicBoomDefaultOrientation;

	UPROPERTY(EditDefaultsOnly, Category = SonicBoomEffect)
		FRotator SonicBoomDiveOrientation;

	UPROPERTY(EditDefaultsOnly, Category = SonicBoomEffect)
		FRotator SonicBoomTakeoffOrientation;

	UPROPERTY(EditDefaultsOnly, Category = SonicBoomEffect)
		float SonicBoomSoundStartTime = 1.f;

	//Dive

	UPROPERTY(EditDefaultsOnly, Category = DiveEffect)
		UParticleSystem* DiveTrailEffect;

	UPROPERTY(EditDefaultsOnly, Category = DiveEffect)
		UNiagaraSystem* DiveLandEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = DiveEffect)
		USoundBase* DiveLandSound;

		
	UParticleSystemComponent* SetupParticleSystemComponent(UParticleSystem* ParticleTemplate);

	UNiagaraComponent* SetupNiagaraComponent(UNiagaraSystem* NiagaraSystemAsset);

	UAudioComponent* SetupAudioComponent(USoundBase* AudioSound);
};
