// Copyright Epic Games, Inc. All Rights Reserved.

#include "SteelheartGameMode.h"
#include "SteelheartCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASteelheartGameMode::ASteelheartGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
