// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SteelheartCharacter.generated.h"

UCLASS(config = Game)
class ASteelheartCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;
	
public:
	ASteelheartCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	UPROPERTY(BlueprintReadOnly, Category = Locomotion)
		bool bIsDashing;
	
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
	void Fly();

	void StopFlying();

	virtual void Landed(const FHitResult& Hit) override;

	virtual void OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal, const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta) override;
	
	void Dash();

	void StopDashing();

	void ProcessDashLerp(float DeltaSeconds);

	void InverseDashLerp();

	bool CheckAngleBetweenVelocityAndRightVector();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
	UPROPERTY(EditDefaultsOnly, Category = BasicFlight)
		float RemnantFallVelocityCoeffOnFly = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category = BasicFlight)
		float RemnantFallVelocityCap = 700.f;

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

	UPROPERTY(EditDefaultsOnly, Category = Dashing)
		float WalkDashSpeed = 3000.f;

	UPROPERTY(EditDefaultsOnly, Category = Dashing)
		float FlyDashSpeed = 3000.f;
	
	UPROPERTY(EditDefaultsOnly, Category = Dashing)
		float DashAcceleration = 50000.f;

	UPROPERTY(EditDefaultsOnly, Category = Dashing)
		float DashLerpTime = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = Dashing)
		float CameraBoomDashLength = 600.f;
	
	float LandingInitiationLocationZ;
		
	float WalkBaseSpeed;

	float FlyBaseSpeed;
	
	float BaseAcceleration;

	float CameraBoomBaseLength;

	float DashLerpTimeCounter = 0.f;

	float DashLerpAlpha = 0.f;

	bool bProcessDashLerp;

	bool bProcessStopDashLerp;
};