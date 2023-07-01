#pragma once

#include "CoreMinimal.h"
#include "FlightComponent.h"
#include "FlightLocomotionComponent.generated.h"

// Macro defining a scaling factor for divebomb rate
#define DIVEBOMB_RATE_SCALE 2.f

// Delegate for initiated divebomb event
DECLARE_DELEGATE(FInitiatedDivebomb);

// Delegate for end of divebomb landing event
DECLARE_DELEGATE_OneParam(FEndDivebombLand, FVector);

UCLASS(ClassGroup = (FlightLocomotion))
class STEELHEART_API UFlightLocomotionComponent : public UFlightComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFlightLocomotionComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Flight actions
	void Fly();

	void StopFlying();

	void Dash();

	void StopDashing();

	void RightDodge();

	void LeftDodge();

	/**
	 * Handles character landing after a jump or flight.
	 *
	 * @param Hit Result describing the landing that resulted in a valid landing spot.
	 * @return Returns true if landing was hard, otherwise returns false.
	 */
	bool HandleCharacterLanding(const FHitResult& Hit);

	void SetLandingInitiationLocationZ(float Value);

	void StopDivebomb();

	// Getter for divebomb initiation delegate
	FORCEINLINE FInitiatedDivebomb* GetDivebombInitiateDelegate() { return &OnInitiateDivebomb; }

	// Getter for divebomb land end delegate
	FORCEINLINE FEndDivebombLand* GetDivebombLandEndDelegate() { return &OnDivebombLandEnd; }

	// Getter for dodging state
	FORCEINLINE bool GetIsDodging() { return bIsDodging; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	// Update flight locomotion
	void UpdateFlightLocomotion(float DeltaTime);

	// Update rotation during flight
	void UpdateRotation(float DeltaTime);

	// Update blend rates
	void UpdateBlendRates();

	// Apply dodge force during dodging
	void ApplyDodgeForce(float DeltaTime);

	// Smoothly reset the pitch of the character during flight
	void SmoothResetPitch(float DeltaTime);

	// Initiate the start of a divebomb action
	void InitiateDivebombStart();

	// Update divebomb action
	void UpdateDivebomb(float DeltaTime);

	// Reset the dodging state
	UFUNCTION()
		void ResetDodge();

	// Reset the dodge timer
	UFUNCTION()
		void ResetDodgeTimer();

	// Initiate the divebomb action
	UFUNCTION()
		void InitiateDivebomb();

	// End the divebomb landing
	UFUNCTION()
		void EndDivebombLand();

public:
	// Rotation rate around the X-axis
	UPROPERTY(BlueprintReadOnly, Category = AnimationHandling)
		float XRotationRate;

	// Rotation rate around the Y-axis
	UPROPERTY(BlueprintReadOnly, Category = AnimationHandling)
		float YRotationRate;

	// Dodging state for right dodge
	UPROPERTY(BlueprintReadOnly, Category = FlightLocomotion)
		bool bIsDodgingRight;

	// Dodging state for left dodge
	UPROPERTY(BlueprintReadOnly, Category = FlightLocomotion)
		bool bIsDodgingLeft;

private:
	// Montage for hard landing
	UPROPERTY(EditDefaultsOnly, Category = FlightLanding)
		UAnimMontage* HardLandingMontage = nullptr;

	// Montage for divebomb action
	UPROPERTY(EditDefaultsOnly, Category = Divebomb)
		UAnimMontage* DivebombMontage = nullptr;

	// Section name in the divebomb montage for landing
	UPROPERTY(EditDefaultsOnly, Category = Divebomb)
		FName DiveMontageLandSectionName = "Land";

	// Height buffer for divebomb engagement
	UPROPERTY(EditDefaultsOnly, Category = Divebomb)
		float DiveEngageHeightBuffer = 2400.f;

	// Floor check trace ratio for divebomb engagement
	UPROPERTY(EditDefaultsOnly, Category = Divebomb)
		float DiveEngageFloorCheckTraceRatio = 1.5f;

	// Floor check trace ratio for divebomb landing
	UPROPERTY(EditDefaultsOnly, Category = Divebomb)
		float DiveLandFloorCheckTraceRatio = 20.f;

	// Velocity for divebomb action
	UPROPERTY(EditDefaultsOnly, Category = Divebomb)
		float DivebombVelocity = 65000.f;

	// Soft landing limit
	UPROPERTY(EditDefaultsOnly, Category = FlightLanding)
		float SoftLandingLimit = 1500.f;

	// Base flight speed
	UPROPERTY(EditDefaultsOnly, Category = FlightLocomotion)
		float BaseSpeed = 850.f;

	// Dash speed
	UPROPERTY(EditDefaultsOnly, Category = FlightLocomotion)
		float DashSpeed = 9000.f;

	// Base acceleration
	UPROPERTY(EditDefaultsOnly, Category = FlightLocomotion)
		float BaseAcceleration = 2500.f;

	// Dash acceleration
	UPROPERTY(EditDefaultsOnly, Category = FlightLocomotion)
		float DashAcceleration = 80000.f;

	// Base dodge force
	UPROPERTY(EditDefaultsOnly, Category = FlightLocomotion)
		float BaseDodgeForce = 32000000.f;

	// Braking deceleration while flying
	UPROPERTY(EditDefaultsOnly, Category = FlightLocomotion)
		float BrakingDecelerationFlying = 2800.f;

	// Interpolation speed for rotation
	UPROPERTY(EditDefaultsOnly, Category = FlightLocomotionRatios)
		float RotationInterpSpeed = 8.f;

	// Interpolation speed for dodging
	UPROPERTY(EditDefaultsOnly, Category = FlightLocomotionRatios)
		float DodgeInterpSpeed = 8.f;

	// Z-axis momentum coefficient
	UPROPERTY(EditDefaultsOnly, Category = FlightLocomotionRatios)
		float ZMomentumCoeff = 0.7f;

	// Duration of the dodge animation
	UPROPERTY(EditDefaultsOnly, Category = AnimationHandling)
		float DodgeTime = 1.5f;

	// Buffer time after dodge for resetting dodge state
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
