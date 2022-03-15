// Fill out your copyright notice in the Description page of Project Settings.


#include "Steelheart/Animation/AnimNotifies/Public/FlightLandingAnimNotify.h"
#include "Kismet/GameplayStatics.h"
#include "Steelheart/Characters/Public/SteelheartCharacter.h"

void UFlightLandingAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	ASteelheartCharacter* SteelheartCharacter = Cast<ASteelheartCharacter>(MeshComp->GetOwner());
	if (ensure(SteelheartCharacter != nullptr))
	{
		SteelheartCharacter->EnableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	}
}