// Copyright Epic Games, Inc. All Rights Reserved.

#include "Steelheart/Characters/Public/SteelheartCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SphereComponent.h"
#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemObjects.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
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

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	GetCharacterMovement()->BrakingDecelerationFlying = 1500.f;
	
	RunSpeed = GetCharacterMovement()->MaxWalkSpeed;
	BaseAcceleration = GetCharacterMovement()->GetMaxAcceleration();
	BaseJumpZVelocity = GetCharacterMovement()->JumpZVelocity;
	MaxSpeedTarget = RunSpeed;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 450.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	CameraBoomBaseLength = CameraBoom->TargetArmLength;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset (to avoid direct content references in C++)

	FlightLocomotion = CreateDefaultSubobject<UFlightLocomotionComponent>(TEXT("FlightLocomotionComponent"));
	FlightLocomotion->GetDivebombInitiateDelegate()->BindUFunction(this, "Dive");
	FlightLocomotion->GetDivebombLandEndDelegate()->BindUFunction(this, "LandDive");

	FlightTakeoff = CreateDefaultSubobject<UFlightTakeoffComponent>(TEXT("FlightTakeoffComponent"));
	FlightTakeoff->GetTakeoffReleaseDelegate()->BindUFunction(this, "ReleaseTakeoff");

	FlightEffects = CreateDefaultSubobject<UFlightEffectsComponent>(TEXT("FlightEffectsComponent"));
	
	FlightCollision = CreateDefaultSubobject<UFlightCollisionComponent>(TEXT("FlightCollisionComponent"));
	InitializeCollision();

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

	bool NoInput = FMath::IsNearlyZero(FrameInputs.X) && FMath::IsNearlyZero(FrameInputs.Y)
		&& FMath::IsNearlyZero(FrameInputs.Z);

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
	
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASteelheartCharacter::HandleFlyInput);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &ASteelheartCharacter::HandleDashInput);

	PlayerInputComponent->BindAction("TakeOff", IE_Pressed, this, &ASteelheartCharacter::HandleTakeoffEngageInput);
	PlayerInputComponent->BindAction("TakeOff", IE_Released, FlightTakeoff, &UFlightTakeoffComponent::ReleaseTakeOff);

	PlayerInputComponent->BindAction("Walk", IE_Pressed, this, &ASteelheartCharacter::Walk);
	PlayerInputComponent->BindAction("Walk", IE_Released, this, &ASteelheartCharacter::StopWalking);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASteelheartCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASteelheartCharacter::MoveRight);
	PlayerInputComponent->BindAxis("MoveUp", this, &ASteelheartCharacter::MoveUp);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASteelheartCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASteelheartCharacter::LookUpAtRate);
}

void ASteelheartCharacter::HandleFlyInput()
{
	if (bLocomotionEnabled)
	{
		if (GetCharacterMovement()->IsFlying()) //Stop Flying
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
		else if (GetCharacterMovement()->IsFalling()) //Fly
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
		else //Jump
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
				Direction = FollowCamera->GetForwardVector();
			}
			else
			{
				// find out which way is forward
				const FRotator Rotation = Controller->GetControlRotation();
				const FRotator YawRotation(0, Rotation.Yaw, 0);

				// get forward vector
				Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			}

			AddMovementInput(Direction, Value);
		}
	}

	FrameInputs.X = Value;
}

void ASteelheartCharacter::MoveRight(float Value)
{
	if (bLocomotionEnabled)
	{
		if ((Controller != nullptr) && (Value != 0.0f))
		{
			// find out which way is right
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get right vector 
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			if (GetCharacterMovement()->IsFlying() && bIsDashing)
			{
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
			else // add movement in that direction
			{
				AddMovementInput(Direction, Value);
			}
		}
	}

	FrameInputs.Y = FMath::Abs(Value);
}

void ASteelheartCharacter::MoveUp(float Value)
{
	if (bLocomotionEnabled)
	{
		if ((Controller != nullptr) && (Value != 0.0f))
		{
			// add movement in upwards direction
			AddMovementInput(FVector::UpVector, Value);
		}
	}

	FrameInputs.Z = FMath::Abs(Value);
}

void ASteelheartCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASteelheartCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ASteelheartCharacter::InitializeCollision()
{
	FieldSystem = CreateDefaultSubobject<UFieldSystemComponent>(TEXT("FieldSystemComponent"));
	FieldSystem->SetupAttachment(RootComponent);
	FlightCollision->SetFieldSystem(FieldSystem);

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(FieldSystem);
	FlightCollision->SetCollisionSphere(CollisionSphere);

	RadialFalloff = CreateDefaultSubobject<URadialFalloff>(TEXT("RadialFalloff"));
	FlightCollision->SetRadialFalloff(RadialFalloff);

	RadialVector = CreateDefaultSubobject<URadialVector>(TEXT("RadialVector"));
	FlightCollision->SetRadialVector(RadialVector);

	CullingField = CreateDefaultSubobject<UCullingField>(TEXT("CullingField"));
	FlightCollision->SetCullingField(CullingField);
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

	FVector CapsuleAngularVelocity = GetCapsuleComponent()->GetPhysicsAngularVelocityInDegrees();
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
		SpeedWhenStopping = CurrentSpeed;
		bRecordedStoppingSpeed = true;
	}
}

void ASteelheartCharacter::UpdateSpeeds(float DeltaSeconds)
{
	CurrentSpeed = GetVelocity().Size();

	float MaxGroundSpeed = FMath::FInterpTo(GetCharacterMovement()->MaxWalkSpeed, MaxSpeedTarget, DeltaSeconds, MaxGroundSpeedInterpSpeed);
	GetCharacterMovement()->MaxWalkSpeed = MaxGroundSpeed;
}

//////////////////////////////////////////////////////////////////////////
// Walk Handling

void ASteelheartCharacter::Walk()
{
	MaxSpeedTarget = WalkSpeed;
}

void ASteelheartCharacter::StopWalking()
{
	MaxSpeedTarget = RunSpeed;
}

//////////////////////////////////////////////////////////////////////////
// Dash Handling

void ASteelheartCharacter::Dash()
{
	bIsDashing = true;
	FlightEffects->ActivateSonicBoom();
	FlightEffects->ToggleDashTrail(true);

	if (GetCharacterMovement()->IsFlying())
	{
		FlightLocomotion->Dash();
	}
	else
	{
		GetCharacterMovement()->MaxAcceleration = DashAcceleration;
	}

	MaxSpeedTarget = DashSpeed;
	GetCharacterMovement()->JumpZVelocity = DashJumpZVelocity;

	CameraBoomLerpTime = DashLerpTime;
	StartCameraBoomLerp();
}

void ASteelheartCharacter::StopDashing()
{
	bIsDashing = false;
	FlightEffects->ToggleDashTrail(false);

	if (GetCharacterMovement()->IsFlying())
	{
		FlightLocomotion->StopDashing();
	}
	else
	{
		GetCharacterMovement()->MaxAcceleration = BaseAcceleration;
	}

	MaxSpeedTarget = RunSpeed;
	GetCharacterMovement()->JumpZVelocity = BaseJumpZVelocity;

	StopCameraBoomLerp();
}

// //////////////////////////////////////////////////////////////////////////
// Landing Handling

void ASteelheartCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (FlightLocomotion->HandleCharacterLanding(Hit))
	{
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

	FlightLocomotion->SetLandingInitiationLocationZ(PreviousLocation.Z);
}

void ASteelheartCharacter::NotifyJumpApex()
{
	Super::NotifyJumpApex();

	FlightLocomotion->SetLandingInitiationLocationZ(GetActorLocation().Z);
}

void ASteelheartCharacter::Dive()
{
	FlightEffects->ActivateDiveTrail();

	CameraBoomLerpTime = DiveLerpTime;
	StartCameraBoomLerp();
}

void ASteelheartCharacter::LandDive(FVector LandLocation)
{
	FlightEffects->ActivateDiveLand(LandLocation);

	StopCameraBoomLerp();
}

void ASteelheartCharacter::ReleaseTakeoff(bool Activate)
{
	if (Activate)
	{
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;

		FlightLocomotion->Fly();
	}

	FlightEffects->ToggleTakeOffCharge(false, Activate);
}

void ASteelheartCharacter::StartCameraBoomLerp()
{
	bProcessDashLerp = true;
	if (bProcessStopDashLerp)
	{
		bProcessStopDashLerp = false;
		InverseCameraBoomLerp();
	}
}

void ASteelheartCharacter::StopCameraBoomLerp()
{
	bProcessStopDashLerp = true;
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
	float InitialBoomLength, TargetBoomLength;

	if (bProcessDashLerp)
	{
		InitialBoomLength = CameraBoomBaseLength;
		TargetBoomLength = CameraBoomTargetLength;
	}
	else if (bProcessStopDashLerp)
	{
		InitialBoomLength = CameraBoomTargetLength;
		TargetBoomLength = CameraBoomBaseLength;
	}
	
	CameraBoomLerpTimeCounter += DeltaSeconds;
	CameraBoomLerpAlpha = CameraBoomLerpTimeCounter / CameraBoomLerpTime;
	
	CameraBoom->TargetArmLength = FMath::Lerp(InitialBoomLength, TargetBoomLength, CameraBoomLerpAlpha);

	if (CameraBoomLerpAlpha >= 1.f)
	{
		bProcessDashLerp = false;
		bProcessStopDashLerp = false;
		
		CameraBoom->TargetArmLength = TargetBoomLength;

		CameraBoomLerpTimeCounter = 0.f;
		CameraBoomLerpAlpha = 0.f;
	}
}

void ASteelheartCharacter::InverseCameraBoomLerp()
{
	CameraBoomLerpTimeCounter = DashLerpTime - CameraBoomLerpTimeCounter;
	CameraBoomLerpAlpha = 1.f - CameraBoomLerpAlpha;
}