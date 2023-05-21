// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlightComponent.h"
#include "FlightTakeoffComponent.generated.h"

#define RATE_SCALE 1.f

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

	void EngageTakeOff();

	void ReleaseTakeOff();

	FORCEINLINE FReleasedTakeoff* GetTakeoffReleaseDelegate() { return &OnReleaseTakeoff; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
private:
	UFUNCTION()
		void LoopTakeOff();

	UFUNCTION()
		void EndTakeOff();

	UPROPERTY(EditDefaultsOnly, Category = TakeOffAnimation)
		UAnimMontage* TakeOffMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = TakeOffAnimation)
		FName LoopSectionName = "ChargeLoop";

	UPROPERTY(EditDefaultsOnly, Category = TakeOffAnimation)
		FName ReleaseSectionName = "TakeOff";

	UPROPERTY(EditDefaultsOnly)
		float BaseTakeOffForce = 3500000.f;

	FTimerHandle TakeOffLoopTimerHandle;

	FTimerHandle TakeOffEndTimerHandle;
	
	FTimerDelegate TakeOffLoopTimerDelegate;

	FTimerDelegate TakeOffEndTimerDelegate;

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
