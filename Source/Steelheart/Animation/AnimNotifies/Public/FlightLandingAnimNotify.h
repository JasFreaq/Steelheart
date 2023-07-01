// Copyright notice

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "FlightLandingAnimNotify.generated.h"

/**
 * Custom animation notify for flight landing.
 */
UCLASS()
class STEELHEART_API UFlightLandingAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	/**
	 * Function called when the animation reaches this notify.
	 *
	 * @param MeshComp The skeletal mesh component playing the animation.
	 * @param Animation The animation sequence being played.
	 */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};