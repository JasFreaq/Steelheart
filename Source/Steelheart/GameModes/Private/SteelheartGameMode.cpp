// Copyright Epic Games, Inc. All Rights Reserved.

#include "Steelheart/GameModes/Public/SteelheartGameMode.h"
#include "UObject/ConstructorHelpers.h"

ASteelheartGameMode::ASteelheartGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/_Game/Blueprints/Characters/BP_SteelheartCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
