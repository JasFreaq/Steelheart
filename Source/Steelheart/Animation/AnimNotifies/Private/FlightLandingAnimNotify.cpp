// Fill out your copyright notice in the Description page of Project Settings.


#include "Steelheart/Animation/AnimNotifies/Public/FlightLandingAnimNotify.h"
#include "Kismet/GameplayStatics.h"
#include "Steelheart/Characters/Public/SteelheartCharacter.h"

void UFlightLandingAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (ensure(OwnerCharacter != nullptr))
	{
		OwnerCharacter->EnableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	}
}