// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlightComponent.h"
#include "FlightEffectsComponent.generated.h"

class UParticleSystemComponent;
class UNiagaraComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class STEELHEART_API UFlightEffectsComponent : public UFlightComponent
{
	GENERATED_BODY()

	UParticleSystemComponent* SonicBoomParticles;

	UParticleSystemComponent* DiveTrailParticles;
	
	UNiagaraComponent* DashTrailNiagara;

public:	
	// Sets default values for this component's properties
	UFlightEffectsComponent();

	FORCEINLINE void SetSonicBoom(UParticleSystemComponent* SonicBoom) { SonicBoomParticles = SonicBoom; }

	FORCEINLINE void SetDiveTrail(UParticleSystemComponent* DiveTrail) { DiveTrailParticles = DiveTrail; }

	FORCEINLINE void SetDashTrail(UNiagaraComponent* DashTrail) { DashTrailNiagara = DashTrail; }

	void ActivateSonicBoom();

	UFUNCTION()
		void ActivateDiveTrail();

	UFUNCTION()
		void ActivateDiveLand(FVector LandLocation);

	void ToggleDashTrail(bool Enable);

private:
	UPROPERTY(EditDefaultsOnly, Category = DiveEffects)
		UParticleSystem* DiveLandEffect;

	UPROPERTY(EditDefaultsOnly, Category = DashEffects)
		FRotator SonicBoomDefaultOrientation;

	UPROPERTY(EditDefaultsOnly, Category = DiveEffects)
		FRotator SonicBoomDiveOrientation;

	UPROPERTY(EditDefaultsOnly, Category = TakeoffEffects)
		FRotator SonicBoomTakeoffOrientation;
};
