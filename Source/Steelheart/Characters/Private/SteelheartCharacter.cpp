// Copyright Epic Games, Inc. All Rights Reserved.

#include "Steelheart/Characters/Public/SteelheartCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

//////////////////////////////////////////////////////////////////////////
// ASteelheartCharacter

ASteelheartCharacter::ASteelheartCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

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

	GetCharacterMovement()->BrakingDecelerationFlying = 500.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Lifecycle Functions

void ASteelheartCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	GetCharacterMovement()->bUseControllerDesiredRotation = GetCharacterMovement()->Velocity.Size() > 0.f;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASteelheartCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASteelheartCharacter::HandleJumpInput);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Jump", IE_DoubleClick, this, &ASteelheartCharacter::Fly);

	
	PlayerInputComponent->BindAxis("MoveForward", this, &ASteelheartCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASteelheartCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASteelheartCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASteelheartCharacter::LookUpAtRate);
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

void ASteelheartCharacter::HandleJumpInput()
{
	if (GetCharacterMovement()->IsFlying())
	{
		StopFlying();
	}
	else
	{
		Super::Jump();		
	}
}

void ASteelheartCharacter::Fly()
{
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
}

void ASteelheartCharacter::StopFlying()
{
	GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	LandingInitiationLocationZ = GetActorLocation().Z;
}

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
}

void ASteelheartCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

//////////////////////////////////////////////////////////////////////////
// Locomotion Handling

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
