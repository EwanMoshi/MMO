// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "MMO.h"
#include "MMOGameMode.h"
#include "MMOHUD.h"
#include "MMOCharacter.h"
#include "MMOPlayerController.h"

AMMOGameMode::AMMOGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	PlayerControllerClass = AMMOPlayerController::StaticClass();

	// use our custom HUD class
	HUDClass = AMMOHUD::StaticClass();
}
