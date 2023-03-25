// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlightComponent.h"
#include "FlightLocomotionComponent.generated.h"

#define DIVEBOMB_RATE_SCALE 2.f

DECLARE_DELEGATE(FInitiatedDivebomb);
DECLARE_DELEGATE_OneParam(FEndDivebombLand, FVector);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
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

	/**
	 * @param Hit Result describing the landing that resulted in a valid landing spot.
	 * @return Returns true if landing was hard, otherwise returns false.
	 */
	bool HandleCharacterLanding(const FHitResult& Hit);

	void SetLandingInitiationLocationZ(float Value);

	void StopDivebomb();

	FORCEINLINE FInitiatedDivebomb* GetDivebombInitiateDelegate() { return &OnInitiateDivebomb; }

	FORCEINLINE FEndDivebombLand* GetDivebombLandEndDelegate() { return &OnDivebombLandEnd; }

	FORCEINLINE bool GetIsDodging() { return bIsDodging; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	void UpdateFlightLocomotion(float DeltaTime);

	void UpdateRotation(float DeltaTime);

	void UpdateBlendRates();

	void ApplyDodgeForce(float DeltaTime);

	void SmoothResetPitch(float DeltaTime);

	void InitiateDivebombStart();

	void UpdateDivebomb(float DeltaTime);

	UFUNCTION()
		void ResetDodge();

	UFUNCTION()
		void ResetDodgeTimer();

	UFUNCTION()
		void InitiateDivebomb();

	UFUNCTION()
		void EndDivebombLand();

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
		UAnimMontage* HardLandingMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = Divebomb)
		UAnimMontage* DivebombMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = Divebomb)
		FName DiveMontageLandSectionName = "Land";

	UPROPERTY(EditDefaultsOnly, Category = Divebomb)
		float DiveEngageHeightBuffer = 2400.f;

	UPROPERTY(EditDefaultsOnly, Category = Divebomb)
		float DiveEngageFloorCheckTraceRatio = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = Divebomb)
		float DiveLandFloorCheckTraceRatio = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = Divebomb)
		float DivebombVelocity = 65000.f;
	
	UPROPERTY(EditDefaultsOnly, Category = FlightLanding)
		float SoftLandingLimit = 1500.f;
	
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

	FTimerHandle DivebombTimerHandle;

	FTimerHandle DivebombLandTimerHandle;

	FTimerDelegate DodgeTimerDelegate;

	FTimerDelegate DodgeResetBufferTimerDelegate;

	FTimerDelegate DivebombTimerDelegate;

	FTimerDelegate DivebombLandTimerDelegate;

	FCollisionQueryParams DivebombTraceParams;

	FInitiatedDivebomb OnInitiateDivebomb;

	FEndDivebombLand OnDivebombLandEnd;
	
	bool bWasDashing;

	bool bIsDodging;

	bool bInitiatedDivebomb;

	bool bIsDivebombing;

	bool bIsLandingDivebomb;

	float CapsuleHalfHeight;

	float CurrentDodgeForce;

	float LandingInitiationLocationZ;

	float DivebombStartSectionLength;

	float DivebombLandSectionLength;
};