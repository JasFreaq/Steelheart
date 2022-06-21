// Copyright Epic Games, Inc. All Rights Reserved.

#include "Steelheart/Characters/Public/SteelheartCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Steelheart/Components/Public/FlightLocomotionComponent.h"

//////////////////////////////////////////////////////////////////////////
// ASteelheartCharacter

ASteelheartCharacter::ASteelheartCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 90.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	GetCharacterMovement()->BrakingDecelerationFlying = 1500.f;
	
	WalkBaseSpeed = GetCharacterMovement()->MaxWalkSpeed;
	WalkBaseAcceleration = GetCharacterMovement()->GetMaxAcceleration();
	
	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	CameraBoomBaseLength = CameraBoom->TargetArmLength;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset (to avoid direct content references in C++)

	FlightLocomotionComponent = CreateDefaultSubobject<UFlightLocomotionComponent>(TEXT("FlightLocomotionComponent"));
}

void ASteelheartCharacter::BeginPlay()
{
	Super::BeginPlay();

	//FlightLocomotionComponent->InitializeFlightLocomotion(this, FollowCamera, GetCapsuleComponent(), GetCharacterMovement());
}

//////////////////////////////////////////////////////////////////////////
// Lifecycle Functions

void ASteelheartCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bProcessDashLerp && CheckAngleBetweenVelocityAndRightVector())
	{
		StopDashing();
	}

	if (bProcessDashLerp || bProcessStopDashLerp)
	{
		ProcessDashLerp(DeltaSeconds);
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
	if (GetCharacterMovement()->IsFlying())
	{
		if (bIsDashing)
		{
			GetCharacterMovement()->MaxAcceleration = WalkDashAcceleration;
		}
		else
		{
			GetCharacterMovement()->MaxAcceleration = WalkBaseAcceleration;
		}

		FlightLocomotionComponent->StopFlying();
	}
	else if (GetCharacterMovement()->IsFalling())
	{
		Super::StopJumping();
		FlightLocomotionComponent->Fly();
	}
	else
	{
		Super::Jump();		
	}
}

void ASteelheartCharacter::HandleDashInput()
{
	if (bIsDashing)
	{
		StopDashing();
	}
	else
	{
		Dash();		
	}
}

//////////////////////////////////////////////////////////////////////////
// Locomotion Handling

void ASteelheartCharacter::MoveForward(float Value)
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

	if (bIsDashing && Value <= 0.8f)
	{
		StopDashing();
	}
}

void ASteelheartCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}

	if (bIsDashing && FMath::Abs(Value) >= 0.8f)
	{
		if (Value >= 0.f)
		{
			FlightLocomotionComponent->RightDodge();
		}
		else
		{
			FlightLocomotionComponent->LeftDodge();
		}
	}
}

void ASteelheartCharacter::MoveUp(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// add movement in upwards direction
		AddMovementInput(GetActorUpVector(), Value);
	}
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

void ASteelheartCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	if (GetCharacterMovement()->IsWalkable(Hit))
	{
		float FloorDistance = LandingInitiationLocationZ - GetActorLocation().Z;

		if (FloorDistance <= SoftLandingUpperLimit)
		{
			if (ensure(SoftLandingMontage != nullptr))
				PlayAnimMontage(SoftLandingMontage);
		}
		else if (FloorDistance > HardLandingLowerLimit)
		{
			if (ensure(HardLandingMontage != nullptr))
				PlayAnimMontage(HardLandingMontage);
		}
		else // Medium Landing
		{
			if (ensure(MediumLandingMontage != nullptr))
				PlayAnimMontage(MediumLandingMontage);
		}
	}
}

void ASteelheartCharacter::OnWalkingOffLedge_Implementation(const FVector& PreviousFloorImpactNormal,
	const FVector& PreviousFloorContactNormal, const FVector& PreviousLocation, float TimeDelta)
{
	LandingInitiationLocationZ = PreviousLocation.Z;
}

void ASteelheartCharacter::Dash()
{
	bIsDashing = true;

	if (GetCharacterMovement()->IsFlying())
	{
		FlightLocomotionComponent->Dash();
	}
	else
	{
		GetCharacterMovement()->MaxAcceleration = WalkDashAcceleration;
	}

	GetCharacterMovement()->MaxWalkSpeed = WalkDashSpeed;
	
	bProcessDashLerp = true;
	if (bProcessStopDashLerp)
	{
		bProcessStopDashLerp = false;
		InverseDashLerp();
	}
}

void ASteelheartCharacter::StopDashing()
{
	bIsDashing = false;

	if (GetCharacterMovement()->IsFlying())
	{
		FlightLocomotionComponent->StopDashing();
	}
	else
	{
		GetCharacterMovement()->MaxAcceleration = WalkBaseAcceleration;
	}

	GetCharacterMovement()->MaxWalkSpeed = WalkBaseSpeed;

	bProcessStopDashLerp = true;
	if (bProcessDashLerp)
	{
		bProcessDashLerp = false;
		InverseDashLerp();
	}
}

void ASteelheartCharacter::ProcessDashLerp(float DeltaSeconds)
{
	float InitialBoomLength, TargetBoomLength;

	if (bProcessDashLerp)
	{
		InitialBoomLength = CameraBoomBaseLength;
		TargetBoomLength = CameraBoomDashLength;
	}
	else if (bProcessStopDashLerp)
	{
		InitialBoomLength = CameraBoomDashLength;
		TargetBoomLength = CameraBoomBaseLength;
	}
	
	DashLerpTimeCounter += DeltaSeconds;
	DashLerpAlpha = DashLerpTimeCounter / DashLerpTime;
	
	CameraBoom->TargetArmLength = FMath::Lerp(InitialBoomLength, TargetBoomLength, DashLerpAlpha);

	if (DashLerpAlpha >= 1.f)
	{
		bProcessDashLerp = false;
		bProcessStopDashLerp = false;
		
		CameraBoom->TargetArmLength = TargetBoomLength;

		DashLerpTimeCounter = 0.f;
		DashLerpAlpha = 0.f;
	}
}

void ASteelheartCharacter::InverseDashLerp()
{
	DashLerpTimeCounter = DashLerpTime - DashLerpTimeCounter;
	DashLerpAlpha = 1.f - DashLerpAlpha;
}

bool ASteelheartCharacter::CheckAngleBetweenVelocityAndRightVector()
{
	FVector Velocity = GetCharacterMovement()->Velocity;

	float DotProduct = FVector::DotProduct(Velocity, GetActorRightVector());
	float Theta = FMath::Acos(DotProduct / Velocity.Size());
	bool IsValid = FMath::RadiansToDegrees(Theta) < 45.f;

	DotProduct = FVector::DotProduct(Velocity, -GetActorRightVector());
	Theta = FMath::Acos(DotProduct / Velocity.Size());
	IsValid = IsValid || FMath::RadiansToDegrees(Theta) < 45.f;

	return IsValid;
}
