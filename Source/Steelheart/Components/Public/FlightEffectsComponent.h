// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlightComponent.h"
#include "FlightEffectsComponent.generated.h"

class UParticleSystemComponent;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
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

	FORCEINLINE void SetSonicBoom(UParticleSystemComponent* SonicBoom) { SonicBoomParticles = SonicBoom; }

	FORCEINLINE void SetDiveTrail(UParticleSystemComponent* DiveTrail) { DiveTrailParticles = DiveTrail; }

	FORCEINLINE void SetTakeoffCharge(UParticleSystemComponent* TakeoffCharge) { TakeoffChargeParticles = TakeoffCharge; }

	FORCEINLINE void SetHover(UNiagaraComponent* Hover) { HoverNiagara = Hover; }

	FORCEINLINE void SetDashTrail(UNiagaraComponent* DashTrail) { DashTrailNiagara = DashTrail; }

	FORCEINLINE void SetWindAudio(UAudioComponent* Wind) { WindAudio = Wind; }

	void ActivateSonicBoom();

	void ActivateHover();

	void ActivateDodge(bool Right);

	void ActivateDiveTrail();

	void ActivateHardLanding(FVector LandLocation);

	void ActivateDiveLand(FVector LandLocation);

	void ToggleDashTrail(bool Enable);

	void ToggleTakeOffCharge(bool Enable, bool Activate = false);

private:
	UPROPERTY(EditDefaultsOnly, Category = FlightEffects)
		UNiagaraSystem* HoverEffect;

	UPROPERTY(EditDefaultsOnly, Category = FlightEffects)
		UNiagaraSystem* DodgeEffect;

	UPROPERTY(EditDefaultsOnly, Category = DiveEffects)
		UNiagaraSystem* HardLandingEffect;

	UPROPERTY(EditDefaultsOnly, Category = DiveEffects)
		UNiagaraSystem* DiveLandEffect;

	UPROPERTY(EditDefaultsOnly, Category = DashEffects)
		USoundBase* SonicBoomSound;

	UPROPERTY(EditDefaultsOnly, Category = DiveEffects)
		USoundBase* LandSound;
		
	UPROPERTY(EditDefaultsOnly, Category = DashEffects)
		FRotator SonicBoomDefaultOrientation;

	UPROPERTY(EditDefaultsOnly, Category = DiveEffects)
		FRotator SonicBoomDiveOrientation;

	UPROPERTY(EditDefaultsOnly, Category = TakeoffEffects)
		FRotator SonicBoomTakeoffOrientation;

	UPROPERTY(EditDefaultsOnly, Category = DashEffects)
		float SonicBoomSoundStartTime = 1.f;
};
