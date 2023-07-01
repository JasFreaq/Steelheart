// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Steelheart/Interfaces/Public/FlightLocomotionInterface.h"
#include "SteelheartCharacter.generated.h"

UCLASS(config = Game)
class ASteelheartCharacter : public ACharacter, public IFlightLocomotionInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* FollowCamera;

	/** Flight locomotion */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = FlightLocomotion, meta = (AllowPrivateAccess = "true"))
		class UFlightLocomotionComponent* FlightLocomotion;

	/** Flight effects */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = FlightLocomotion, meta = (AllowPrivateAccess = "true"))
		class UFlightEffectsComponent* FlightEffects;

	/** Flight takeoff */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = FlightLocomotion, meta = (AllowPrivateAccess = "true"))
		class UFlightTakeoffComponent* FlightTakeoff;

	/** Flight collision */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = FlightLocomotion, meta = (AllowPrivateAccess = "true"))
		class UFlightCollisionComponent* FlightCollision;

public:
	ASteelheartCharacter();

	// Override function from IFlightLocomotionInterface to get the camera component
	FORCEINLINE virtual UCameraComponent* GetCameraComponent() override { return FollowCamera; }

	// Override function from IFlightLocomotionInterface to check if character is dashing
	FORCEINLINE virtual bool IsDashing() override { return bIsDashing; }

	// Override function from IFlightLocomotionInterface to set locomotion enabled or disabled
	FORCEINLINE virtual void SetLocomotionEnabled(bool Enabled) override { bLocomotionEnabled = Enabled; }

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	void HandleFlyInput();

	void HandleDashInput();

	void HandleTakeoffEngageInput();

	/** Called for forwards/backwards input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** Called for upwards/downwards input */
	void MoveUp(float Value);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

private:
	// Update the character's locomotion based on the given time
	void UpdateLocomotion(float DeltaSeconds);

	// Update the blend rate for locomotion animations
	void UpdateBlendRate();

	// Record the speed when the character starts to stop
	void RecordStoppingSpeed();

	// Update the character's speeds based on the given time
	void UpdateSpeeds(float DeltaSeconds);

	// Move the character in a walking state
	void Walk();

	// Stop the character from walking
	void StopWalking();

	// Initiate a dash action
	void Dash();

	// Stop the character from dashing
	void StopDashing();

	// Called when the character has landed on the ground
	virtual void Landed(const FHitResult& Hit) override;

	// Called when the character is walking off a ledge
	virtual void OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal, const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta) override;

	// Called when the character has reached the apex of a jump
	virtual void NotifyJumpApex() override;

	// Handle the dive action
	UFUNCTION()
		void Dive();

	// Handle the landing after a dive
	UFUNCTION()
		void LandDive(FVector LandLocation);

	// Release the takeoff action, activating or deactivating it
	UFUNCTION()
		void ReleaseTakeoff(bool Activate);

	// Start the camera boom lerp
	void StartCameraBoomLerp();

	// Stop the camera boom lerp
	void StopCameraBoomLerp();

	// Process the camera boom lerp based on the given time
	void ProcessCameraBoomLerp(float DeltaSeconds);

	// Inverse the camera boom lerp
	void InverseCameraBoomLerp();

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	// Speed required for the character to perform a leap
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Leaping)
		float SpeedRequiredForLeap = 2500.f;

	UPROPERTY(BlueprintReadOnly, Category = Locomotion)
		float CurrentRotationRate;

	UPROPERTY(BlueprintReadOnly, Category = Locomotion)
		float CurrentSpeed;

	UPROPERTY(BlueprintReadOnly, Category = Locomotion)
		float SpeedWhenStopping;

	UPROPERTY(BlueprintReadOnly, Category = Locomotion)
		bool bIsDashing;

private:
	// Animation montage to play when the character starts a regular jump
	UPROPERTY(EditDefaultsOnly, Category = JumpAnimations)
		UAnimMontage* JumpStartMontage = nullptr;

	// Animation montage to play when the character starts a leap
	UPROPERTY(EditDefaultsOnly, Category = JumpAnimations)
		UAnimMontage* LeapStartMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = Walking)
		float WalkSpeed = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = LocomotionRatios)
		float MaxGroundSpeedInterpSpeed = 4.f;

	UPROPERTY(EditDefaultsOnly, Category = Dashing)
		float DashJumpZVelocity = 1200.f;

	UPROPERTY(EditDefaultsOnly, Category = Dashing)
		float DashSpeed = 3000.f;

	UPROPERTY(EditDefaultsOnly, Category = Dashing)
		float DashAcceleration = 50000.f;

	UPROPERTY(EditDefaultsOnly, Category = CameraBoomLerp)
		float DashLerpTime = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = CameraBoomLerp)
		float DiveLerpTime = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category = CameraBoomLerp)
		float CameraBoomTargetLength = 900.f;

	FVector FrameInputs;

	float MaxSpeedTarget;

	float RunSpeed;

	float BaseAcceleration;

	float BaseJumpZVelocity;

	float CameraBoomBaseLength;

	float CameraBoomLerpTime;

	float CameraBoomLerpTimeCounter = 0.f;

	float CameraBoomLerpAlpha = 0.f;

	bool bRecordedStoppingSpeed;

	bool bProcessDashLerp;

	bool bProcessStopDashLerp;

	bool bLocomotionEnabled;
};
