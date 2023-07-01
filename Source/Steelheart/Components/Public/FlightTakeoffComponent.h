#pragma once

#include "CoreMinimal.h"
#include "FlightComponent.h"
#include "FlightTakeoffComponent.generated.h"

// Scale factor for the rate of takeoff
#define RATE_SCALE 1.f

// Delegate for the released takeoff event
DECLARE_DELEGATE_OneParam(FReleasedTakeoff, bool);

UCLASS(ClassGroup = (FlightLocomotion))
class STEELHEART_API UFlightTakeoffComponent : public UFlightComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFlightTakeoffComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Engage the takeoff process
	void EngageTakeOff();

	// Release the takeoff
	void ReleaseTakeOff();

	// Get the delegate for the released takeoff event
	FORCEINLINE FReleasedTakeoff* GetTakeoffReleaseDelegate() { return &OnReleaseTakeoff; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	// Loop the takeoff animation
	UFUNCTION()
		void LoopTakeOff();

	// End the takeoff process
	UFUNCTION()
		void EndTakeOff();

	// Animation montage for takeoff
	UPROPERTY(EditDefaultsOnly, Category = TakeOffAnimation)
		UAnimMontage* TakeOffMontage = nullptr;

	// Name of the loop section within the takeoff montage
	UPROPERTY(EditDefaultsOnly, Category = TakeOffAnimation)
		FName LoopSectionName = "ChargeLoop";

	// Name of the release section within the takeoff montage
	UPROPERTY(EditDefaultsOnly, Category = TakeOffAnimation)
		FName ReleaseSectionName = "TakeOff";

	// Base force applied during takeoff
	UPROPERTY(EditDefaultsOnly)
		float BaseTakeOffForce = 3500000.f;

	FTimerHandle TakeOffLoopTimerHandle;

	FTimerHandle TakeOffEndTimerHandle;

	FTimerDelegate TakeOffLoopTimerDelegate;

	FTimerDelegate TakeOffEndTimerDelegate;

	// Delegate for the released takeoff event
	FReleasedTakeoff OnReleaseTakeoff;

	float BaseBlendOutTime;

	float EngageSectionLength;

	float LoopSectionLength;

	float ReleaseSectionLength;

	float ReleaseTimeCounter;

	float CurrentTakeOffForce;

	bool bIsTakeOffInitiating;

	bool bIsTakeOffLooping;

	bool bIsTakeOffCharged;

	bool bIsTakingOff;
};
