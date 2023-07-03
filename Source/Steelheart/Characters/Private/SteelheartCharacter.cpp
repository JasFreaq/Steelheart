// Include necessary header files
#include "Steelheart/Characters/Public/SteelheartCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Steelheart/Components/Public/FlightCollisionComponent.h"
#include "Steelheart/Components/Public/FlightLocomotionComponent.h"
#include "Steelheart/Components/Public/FlightTakeoffComponent.h"
#include "Steelheart/Components/Public/FlightEffectsComponent.h"

//////////////////////////////////////////////////////////////////////////
// ASteelheartCharacter

ASteelheartCharacter::ASteelheartCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(40.f, 89.0f);

	// Set turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Disable rotation of the character with the controller
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // Rotation rate of the character
	GetCharacterMovement()->JumpZVelocity = 600.f; // Jump velocity
	GetCharacterMovement()->AirControl = 0.2f; // Air control percentage

	GetCharacterMovement()->BrakingDecelerationFlying = 1500.f; // Deceleration when braking in flying mode

	RunSpeed = GetCharacterMovement()->MaxWalkSpeed;
	BaseAcceleration = GetCharacterMovement()->GetMaxAcceleration();
	BaseJumpZVelocity = GetCharacterMovement()->JumpZVelocity;
	MaxSpeedTarget = RunSpeed;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 450.0f; // Distance between camera and character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	CameraBoomBaseLength = CameraBoom->TargetArmLength;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset (to avoid direct content references in C++)

	FlightLocomotion = CreateDefaultSubobject<UFlightLocomotionComponent>(TEXT("FlightLocomotionComponent"));
	FlightLocomotion->GetDivebombInitiateDelegate()->BindUFunction(this, "Dive");
	FlightLocomotion->GetDivebombLandEndDelegate()->BindUFunction(this, "LandDive");

	FlightEffects = CreateDefaultSubobject<UFlightEffectsComponent>(TEXT("FlightEffectsComponent"));

	FlightTakeoff = CreateDefaultSubobject<UFlightTakeoffComponent>(TEXT("FlightTakeoffComponent"));
	FlightTakeoff->GetTakeoffReleaseDelegate()->BindUFunction(this, "ReleaseTakeoff");

	FlightCollision = CreateDefaultSubobject<UFlightCollisionComponent>(TEXT("FlightCollisionComponent"));

	bLocomotionEnabled = true;
}

//////////////////////////////////////////////////////////////////////////
// Lifecycle Functions

void ASteelheartCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCapsuleComponent()->OnComponentHit.AddDynamic(FlightCollision, &UFlightCollisionComponent::OnCharacterHit);
}

void ASteelheartCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateLocomotion(DeltaSeconds);
	
	if (bIsDashing)
	{
		if (GetCharacterMovement()->IsFlying())
		{
			if (FMath::IsNearlyZero(FrameInputs.X) || FrameInputs.X < 0.f)
			{
				StopDashing();
			}
		}
		else
		{
			if (FMath::IsNearlyZero(FrameInputs.X) && FMath::IsNearlyZero(FrameInputs.Y))
			{
				StopDashing();
			}
		}
	}

	if (bProcessDashLerp || bProcessStopDashLerp)
	{
		ProcessCameraBoomLerp(DeltaSeconds);
	}
}


//////////////////////////////////////////////////////////////////////////
// Input

void ASteelheartCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind the "Jump" action to handle flying input
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASteelheartCharacter::HandleFlyInput);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind the "Dash" action to handle dashing input
	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &ASteelheartCharacter::HandleDashInput);

	// Bind the "TakeOff" action to handle takeoff input
	PlayerInputComponent->BindAction("TakeOff", IE_Pressed, this, &ASteelheartCharacter::HandleTakeoffEngageInput);
	PlayerInputComponent->BindAction("TakeOff", IE_Released, FlightTakeoff, &UFlightTakeoffComponent::ReleaseTakeOff);

	// Bind the "Walk" action to handle walking input
	PlayerInputComponent->BindAction("Walk", IE_Pressed, this, &ASteelheartCharacter::Walk);
	PlayerInputComponent->BindAction("Walk", IE_Released, this, &ASteelheartCharacter::StopWalking);

	// Bind the movement axes for character movement
	PlayerInputComponent->BindAxis("MoveForward", this, &ASteelheartCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASteelheartCharacter::MoveRight);
	PlayerInputComponent->BindAxis("MoveUp", this, &ASteelheartCharacter::MoveUp);

	// Bind the rotation axes for character rotation
	PlayerInputComponent->BindAxis("Turn", this, &ASteelheartCharacter::Turn);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASteelheartCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &ASteelheartCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASteelheartCharacter::LookUpAtRate);
}

void ASteelheartCharacter::HandleFlyInput()
{
	if (bLocomotionEnabled)
	{
		if (GetCharacterMovement()->IsFlying()) // Stop Flying
		{
			if (bIsDashing)
			{
				StopDashing();
			}

			bUseControllerRotationYaw = false;
			GetCharacterMovement()->bOrientRotationToMovement = true;

			FlightLocomotion->StopFlying();
			GetCharacterMovement()->bNotifyApex = true;
		}
		else if (GetCharacterMovement()->IsFalling()) // Fly
		{
			Super::StopJumping();

			bUseControllerRotationYaw = true;
			GetCharacterMovement()->bOrientRotationToMovement = false;

			FlightLocomotion->StopDivebomb();
			FlightLocomotion->Fly();

			FlightEffects->ActivateHover();

			if (bIsDashing)
			{
				FlightLocomotion->Dash();
			}
		}
		else // Jump
		{
			Super::Jump();

			GetCharacterMovement()->bNotifyApex = true;
		}
	}
}

void ASteelheartCharacter::HandleDashInput()
{
	if (bLocomotionEnabled)
	{
		if (bIsDashing)
		{
			StopDashing();
		}
		else
		{
			if (GetCharacterMovement()->IsFlying())
			{
				if (FrameInputs.X > 0.f)
				{
					Dash();
				}
			}
			else if (GetCharacterMovement()->IsWalking() &&
				!(FMath::IsNearlyZero(FMath::Abs(FrameInputs.X)) && FMath::IsNearlyZero(FrameInputs.Y)))
			{
				Dash();
			}
		}
	}
}

void ASteelheartCharacter::HandleTakeoffEngageInput()
{
	if (GetCharacterMovement()->IsWalking() &&
		FMath::IsNearlyZero(FrameInputs.X) && FMath::IsNearlyZero(FrameInputs.Y))
	{
		FlightTakeoff->EngageTakeOff();
		FlightEffects->ToggleTakeOffCharge(true);
	}
}


//////////////////////////////////////////////////////////////////////////
// Locomotion Handling

void ASteelheartCharacter::MoveForward(float Value)
{
	if (bLocomotionEnabled)
	{
		if ((Controller != nullptr) && (Value != 0.0f))
		{
			FVector Direction;
			if (GetCharacterMovement()->IsFlying())
			{
				// If character is flying, move in the direction of the camera's forward vector
				Direction = FollowCamera->GetForwardVector();
			}
			else
			{
				// If character is not flying, move in the direction based on controller rotation
				const FRotator Rotation = Controller->GetControlRotation();
				const FRotator YawRotation(0, Rotation.Yaw, 0);

				// Get forward vector
				Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			}

			// Add movement input in the specified direction
			AddMovementInput(Direction, Value);
		}
	}

	FrameInputs.X = Value; // Record the input value for X-axis movement
}

void ASteelheartCharacter::MoveRight(float Value)
{
	if (bLocomotionEnabled)
	{
		if ((Controller != nullptr) && (Value != 0.0f))
		{
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// Get right vector based on controller rotation
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			if (GetCharacterMovement()->IsFlying() && bIsDashing)
			{
				// If character is flying and dashing, perform a dodge maneuver
				if (FMath::Abs(Value) >= 0.8f && !FlightLocomotion->GetIsDodging())
				{
					bool IsRight = Value >= 0.f;

					if (IsRight)
					{
						FlightLocomotion->RightDodge();
					}
					else
					{
						FlightLocomotion->LeftDodge();
					}

					FlightEffects->ActivateDodge(IsRight);
				}
			}
			else
			{
				// Add movement input in the specified direction
				AddMovementInput(Direction, Value);
			}
		}
	}

	FrameInputs.Y = FMath::Abs(Value); // Record the absolute value of input for Y-axis movement
}

void ASteelheartCharacter::MoveUp(float Value)
{
	if (bLocomotionEnabled)
	{
		if ((Controller != nullptr) && (Value != 0.0f))
		{
			// Add movement input in the upward direction
			AddMovementInput(FVector::UpVector, Value);
		}
	}

	FrameInputs.Z = FMath::Abs(Value); // Record the absolute value of input for Z-axis movement
}

void ASteelheartCharacter::Turn(float Value)
{
	// Check if the assigned mouse button is currently pressed
	if (GetWorld()->GetFirstPlayerController()->IsInputKeyDown(MouseRotationKey))
	{
		// Interpolate the current turn value towards the target turn value over time
		float CurrentTurnValue = FMath::FInterpTo(LastTurnValue, Value, GetWorld()->GetDeltaSeconds(), MouseRotationInterpSpeed);

		// Apply the smooth rotation by gradually adjusting the yaw of the character over time
		AddControllerYawInput(CurrentTurnValue * BaseTurnRate * GetWorld()->GetDeltaSeconds());

		// Store the current turn value as the last turn value for interpolation in the next frame
		LastTurnValue = CurrentTurnValue;
	}
}

void ASteelheartCharacter::TurnAtRate(float Rate)
{
	// Add the rotation input for this frame based on the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASteelheartCharacter::LookUp(float Value)
{
	// Check if the assigned mouse button is currently pressed
	if (GetWorld()->GetFirstPlayerController()->IsInputKeyDown(MouseRotationKey))
	{
		// Interpolate the current look-up value towards the target look-up value over time
		float CurrentLookUpValue = FMath::FInterpTo(LastLookUpValue, Value, GetWorld()->GetDeltaSeconds(), MouseRotationInterpSpeed);

		// Apply the smooth rotation by gradually adjusting the pitch of the character over time
		AddControllerPitchInput(CurrentLookUpValue * BaseLookUpRate * GetWorld()->GetDeltaSeconds());

		// Store the current look-up value as the last look-up value for interpolation in the next frame
		LastLookUpValue = CurrentLookUpValue;
	}
}

void ASteelheartCharacter::LookUpAtRate(float Rate)
{
	// Add the rotation input for this frame based on the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());	
}

void ASteelheartCharacter::UpdateLocomotion(float DeltaSeconds)
{
	UpdateBlendRate();

	RecordStoppingSpeed();

	UpdateSpeeds(DeltaSeconds);
}

void ASteelheartCharacter::UpdateBlendRate()
{
	FVector2D InputRange(-360.f, 360.f);
	FVector2D OutputRange(-1.f, 1.f);

	// Get the angular velocity of the character's capsule component
	FVector CapsuleAngularVelocity = GetCapsuleComponent()->GetPhysicsAngularVelocityInDegrees();

	// Map the angular velocity to the desired range
	CurrentRotationRate = FMath::GetMappedRangeValueClamped(InputRange, OutputRange, CapsuleAngularVelocity.Z);
}

void ASteelheartCharacter::RecordStoppingSpeed()
{
	if (GetCharacterMovement()->GetCurrentAcceleration().Size() > 0)
	{
		bRecordedStoppingSpeed = false;
	}
	else if (!bRecordedStoppingSpeed)
	{
		// Record the current speed when the character stops accelerating
		SpeedWhenStopping = CurrentSpeed;
		bRecordedStoppingSpeed = true;
	}
}

void ASteelheartCharacter::UpdateSpeeds(float DeltaSeconds)
{
	// Calculate the current speed of the character
	CurrentSpeed = GetVelocity().Size();

	// Interpolate the maximum ground speed towards the target speed
	float MaxGroundSpeed = FMath::FInterpTo(GetCharacterMovement()->MaxWalkSpeed, MaxSpeedTarget, DeltaSeconds, MaxGroundSpeedInterpSpeed);
	GetCharacterMovement()->MaxWalkSpeed = MaxGroundSpeed;
}

//////////////////////////////////////////////////////////////////////////
// Walk Handling

void ASteelheartCharacter::Walk()
{
	// Set the target maximum speed to the walk speed
	MaxSpeedTarget = WalkSpeed;
}

void ASteelheartCharacter::StopWalking()
{
	// Set the target maximum speed to the run speed
	MaxSpeedTarget = RunSpeed;
}


//////////////////////////////////////////////////////////////////////////
// Dash Handling

void ASteelheartCharacter::Dash()
{
	// Set the character to a dashing state
	bIsDashing = true;

	// Activate visual effects for dashing
	FlightEffects->ActivateSonicBoom();
	FlightEffects->ToggleDashTrail(true);

	if (GetCharacterMovement()->IsFlying())
	{
		// If character is flying, perform a dash maneuver using flight locomotion
		FlightLocomotion->Dash();
	}
	else
	{
		// If character is not flying, set the maximum acceleration to dash acceleration
		GetCharacterMovement()->MaxAcceleration = DashAcceleration;
	}

	// Set the target maximum speed to the dash speed
	MaxSpeedTarget = DashSpeed;
	GetCharacterMovement()->JumpZVelocity = DashJumpZVelocity;

	// Set the camera boom lerp time to the dash lerp time and start the lerping process
	CameraBoomLerpTime = DashCameraLerpTime;
	StartCameraBoomLerp();
}

void ASteelheartCharacter::StopDashing()
{
	// Set the character to a non-dashing state
	bIsDashing = false;

	// Deactivate dash-related visual effects
	FlightEffects->ToggleDashTrail(false);

	if (GetCharacterMovement()->IsFlying())
	{
		// If character is flying, stop the dash maneuver using flight locomotion
		FlightLocomotion->StopDashing();
	}
	else
	{
		// If character is not flying, set the maximum acceleration back to the base acceleration
		GetCharacterMovement()->MaxAcceleration = BaseAcceleration;
	}

	// Set the target maximum speed back to the run speed
	MaxSpeedTarget = RunSpeed;
	GetCharacterMovement()->JumpZVelocity = BaseJumpZVelocity;

	// Stop the camera boom lerping process
	StopCameraBoomLerp();
}


// //////////////////////////////////////////////////////////////////////////
// Landing Handling

void ASteelheartCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (FlightLocomotion->HandleCharacterLanding(Hit))
	{
		// Activate hard landing visual effect
		FlightEffects->ActivateHardLanding(Hit.ImpactPoint);
	}
}

void ASteelheartCharacter::OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal,
	const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta)
{
	if (CurrentSpeed >= SpeedRequiredForLeap)
	{
		if (ensure(LeapStartMontage != nullptr))
			PlayAnimMontage(LeapStartMontage);
	}
	else
	{
		if (ensure(JumpStartMontage != nullptr))
			PlayAnimMontage(JumpStartMontage);
	}

	// Set the landing initiation location Z-coordinate for flight locomotion
	FlightLocomotion->SetLandingInitiationLocationZ(PreviousLocation.Z);
}

void ASteelheartCharacter::NotifyJumpApex()
{
	Super::NotifyJumpApex();

	// Set the landing initiation location Z-coordinate for flight locomotion
	FlightLocomotion->SetLandingInitiationLocationZ(GetActorLocation().Z);
}

void ASteelheartCharacter::Dive()
{
	// Activate dive visual effect
	FlightEffects->ActivateDiveTrail();

	// Set the camera boom lerp time to the dive lerp time and start the lerping process
	CameraBoomLerpTime = DiveCameraLerpTime;
	StartCameraBoomLerp();
}

void ASteelheartCharacter::LandDive(FVector LandLocation)
{
	// Activate landing dive visual effect
	FlightEffects->ActivateDiveLand(LandLocation);

	// Stop the camera boom lerping process
	StopCameraBoomLerp();
}

void ASteelheartCharacter::ReleaseTakeoff(bool Activate)
{
	if (Activate)
	{
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;

		// Activate flight locomotion for takeoff
		FlightLocomotion->Fly();
	}

	// Toggle takeoff charge visual effect
	FlightEffects->ToggleTakeOffCharge(false, Activate);
}

void ASteelheartCharacter::StartCameraBoomLerp()
{
	// Start the camera boom lerping process for dash
	bProcessDashLerp = true;

	// If the stop dash lerping process is already active, inverse it
	if (bProcessStopDashLerp)
	{
		bProcessStopDashLerp = false;
		InverseCameraBoomLerp();
	}
}

void ASteelheartCharacter::StopCameraBoomLerp()
{
	// Start the camera boom lerping process for stop dash
	bProcessStopDashLerp = true;

	// If the dash lerping process is already active, inverse it
	if (bProcessDashLerp)
	{
		bProcessDashLerp = false;
		InverseCameraBoomLerp();
	}
}


//////////////////////////////////////////////////////////////////////////
// Dash Helper Functions

void ASteelheartCharacter::ProcessCameraBoomLerp(float DeltaSeconds)
{
	// Variables to store initial and target lengths of the camera boom
	float InitialBoomLength, TargetBoomLength;

	if (bProcessDashLerp)
	{
		// If dash lerp process is active, set initial boom length to base length and target boom length to target length
		InitialBoomLength = CameraBoomBaseLength;
		TargetBoomLength = CameraBoomTargetLength;
	}
	else if (bProcessStopDashLerp)
	{
		// If stop dash lerp process is active, set initial boom length to target length and target boom length to base length
		InitialBoomLength = CameraBoomTargetLength;
		TargetBoomLength = CameraBoomBaseLength;
	}

	// Increment the lerp time counter based on elapsed time
	CameraBoomLerpTimeCounter += DeltaSeconds;

	// Calculate the alpha value for lerping based on the current lerp time
	CameraBoomLerpAlpha = CameraBoomLerpTimeCounter / CameraBoomLerpTime;

	// Perform linear interpolation to update the target arm length of the camera boom
	CameraBoom->TargetArmLength = FMath::Lerp(InitialBoomLength, TargetBoomLength, CameraBoomLerpAlpha);

	if (CameraBoomLerpAlpha >= 1.f)
	{
		// If the lerp process is complete, reset variables and set the final target arm length
		bProcessDashLerp = false;
		bProcessStopDashLerp = false;

		CameraBoom->TargetArmLength = TargetBoomLength;

		CameraBoomLerpTimeCounter = 0.f;
		CameraBoomLerpAlpha = 0.f;
	}
}

void ASteelheartCharacter::InverseCameraBoomLerp()
{
	// Inverse the lerp time counter and alpha values to reverse the lerp process
	CameraBoomLerpTimeCounter = DashCameraLerpTime - CameraBoomLerpTimeCounter;
	CameraBoomLerpAlpha = 1.f - CameraBoomLerpAlpha;
}