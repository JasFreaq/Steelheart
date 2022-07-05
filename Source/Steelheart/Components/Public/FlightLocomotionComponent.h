// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlightComponent.h"
#include "FlightLocomotionComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STEELHEART_API UFlightLocomotionComponent : public UFlightComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFlightLocomotionComponent();
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Fly();

	void StopFlying();

	void Dash();

	void StopDashing();

	void RightDodge();

	void LeftDodge();

	void HandleCharacterLanding(const FHitResult& Hit);

	void SetLandingInitiationLocationZ(float Value);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	void UpdateFlightLocomotion(float DeltaTime);

	void UpdateFlightRotation(float DeltaTime);

	void UpdateBlendRates();

	void ApplyDodgeForce(float DeltaTime);

	void SmoothResetPitch(float DeltaTime);
		
	UFUNCTION()
		void ResetDodge();

	UFUNCTION()
		void ResetDodgeTimer();

public:
	UPROPERTY(BlueprintReadOnly, Category = AnimationHandling)
		float XRotationRate;

	UPROPERTY(BlueprintReadOnly, Category = AnimationHandling)
		float YRotationRate;
	
	UPROPERTY(BlueprintReadOnly, Category = FlightLocomotion)
		bool bIsDodgingRight;

	UPROPERTY(BlueprintReadOnly, Category = FlightLocomotion)
		bool bIsDodgingLeft;

private:
	UPROPERTY(EditDefaultsOnly, Category = FlightLanding)
		UAnimMontage* SoftLandingMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = FlightLanding)
		UAnimMontage* MediumLandingMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = FlightLanding)
		UAnimMontage* HardLandingMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = FlightLanding)
		float SoftLandingUpperLimit = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = FlightLanding)
		float HardLandingLowerLimit = 1200.f;

	UPROPERTY(EditDefaultsOnly, Category = FlightLocomotion)
		float BaseSpeed = 850.f;

	UPROPERTY(EditDefaultsOnly, Category = FlightLocomotion)
		float DashSpeed = 9000.f;

	UPROPERTY(EditDefaultsOnly, Category = FlightLocomotion)
		float BaseAcceleration = 2500.f;

	UPROPERTY(EditDefaultsOnly, Category = FlightLocomotion)
		float DashAcceleration = 80000.f;

	UPROPERTY(EditDefaultsOnly, Category = FlightLocomotion)
		float BaseDodgeForce = 32000000.f;

	UPROPERTY(EditDefaultsOnly, Category = FlightLocomotion)
		float BrakingDecelerationFlying = 2800.f;

	UPROPERTY(EditDefaultsOnly, Category = FlightLocomotionRatios)
		float RotationInterpSpeed = 8.f;

	UPROPERTY(EditDefaultsOnly, Category = FlightLocomotionRatios)
		float DodgeInterpSpeed = 8.f;

	UPROPERTY(EditDefaultsOnly, Category = FlightLocomotionRatios)
		float ZMomentumCoeff = 0.7f;

	UPROPERTY(EditDefaultsOnly, Category = AnimationHandling)
		float DodgeTime = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = AnimationHandling)
		float DodgeBufferTime = 0.1f;

	FTimerHandle DodgeTimerHandle;

	FTimerHandle DodgeResetBufferTimerHandle;

	FTimerDelegate DodgeTimerDelegate;

	FTimerDelegate DodgeResetBufferTimerDelegate;
	
	bool bWasDashing;

	bool bIsDodging;

	float CapsuleHalfHeight;

	float CurrentDodgeForce;

	float LandingInitiationLocationZ;
};