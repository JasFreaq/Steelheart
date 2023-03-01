// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Steelheart/Interfaces/Public/FlightLocomotionInterface.h"
#include "SteelheartCharacter.generated.h"

class UNiagaraComponent;

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

	/** Flight takeoff */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = FlightLocomotion, meta = (AllowPrivateAccess = "true"))
		class UFlightTakeoffComponent* FlightTakeoff;

	/** Flight effects */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = FlightLocomotion, meta = (AllowPrivateAccess = "true"))
		class UFlightEffectsComponent* FlightEffects;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DashEffects, meta = (AllowPrivateAccess = "true"))
		UParticleSystemComponent* SonicBoomParticles;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DiveEffects, meta = (AllowPrivateAccess = "true"))
		UParticleSystemComponent* DiveTrailParticles;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = TakeoffEffects, meta = (AllowPrivateAccess = "true"))
		UParticleSystemComponent* TakeoffChargeParticles;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = HoverEffects, meta = (AllowPrivateAccess = "true"))
		UNiagaraComponent* HoverNiagara;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DashEffects, meta = (AllowPrivateAccess = "true"))
		UNiagaraComponent* DashTrailNiagara;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = SoundEffects, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* WindAudio;

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
	void InitializeEffects();

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

	UFUNCTION()
		void Dive();

	UFUNCTION()
		void LandDive(FVector LandLocation);

	UFUNCTION()
		void ReleaseTakeoff(bool Activate);

	void StartCameraBoomLerp();

	void StopCameraBoomLerp();

	void ProcessCameraBoomLerp(float DeltaSeconds);

	void InverseCameraBoomLerp();

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