// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Steelheart/Interfaces/Public/FlightLocomotionInterface.h"
#include "SteelheartCharacter.generated.h"

class UFlightLocomotionComponent;
class UFlightTakeoffComponent;

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
		UFlightLocomotionComponent* FlightLocomotion;

	/** Flight takeoff */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = FlightLocomotion, meta = (AllowPrivateAccess = "true"))
		UFlightTakeoffComponent* FlightTakeoff;

public:
	ASteelheartCharacter();
		
	FORCEINLINE virtual UCameraComponent* GetCameraComponent() override { return FollowCamera; }

	FORCEINLINE virtual bool IsDashing() override { return bIsDashing; }

	FORCEINLINE virtual void SetLocomotionEnabled(bool Enabled) override { bLocomotionEnabled = Enabled; }

protected:
	virtual void Tick(float DeltaSeconds) override;

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	void HandleFlyInput();
	
	void HandleDashInput();
			
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
	void UpdateLocomotion(float DeltaSeconds);

	void UpdateBlendRate();

	void RecordStoppingSpeed();

	void UpdateSpeeds(float DeltaSeconds);

	void Walk();

	void StopWalking();

	void Dash();

	void StopDashing();

	virtual void Landed(const FHitResult& Hit) override;

	virtual void OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal, const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta) override;

	virtual void NotifyJumpApex() override;
		
	void ProcessDashLerp(float DeltaSeconds);

	void InverseDashLerp();
	
	bool CheckAngleBetweenVelocityAndRightVector();

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

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
	UPROPERTY(EditDefaultsOnly, Category = JumpAnimations)
		UAnimMontage* JumpStartMontage = nullptr;

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

	UPROPERTY(EditDefaultsOnly, Category = Dashing)
		float DashLerpTime = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = Dashing)
		float CameraBoomDashLength = 600.f;
		
	float MaxSpeedTarget;

	float RunSpeed;
		
	float BaseAcceleration;
	
	float BaseJumpZVelocity;

	float CameraBoomBaseLength;

	float DashLerpTimeCounter = 0.f;

	float DashLerpAlpha = 0.f;

	bool bRecordedStoppingSpeed;

	bool bProcessDashLerp;

	bool bProcessStopDashLerp;

	bool bLocomotionEnabled;
};