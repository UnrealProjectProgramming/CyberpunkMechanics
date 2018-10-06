// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "CyberGameMode.h"
#include "CyberHUD.h"
#include "CyberCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACyberGameMode::ACyberGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ACyberHUD::StaticClass();
}
