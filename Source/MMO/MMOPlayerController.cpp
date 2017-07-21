// Fill out your copyright notice in the Description page of Project Settings.

#include "MMO.h"
#include "MMOPlayerController.h"
#include "MMOPlayerCameraManager.h"

AMMOPlayerController::AMMOPlayerController(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = AMMOPlayerCameraManager::StaticClass();
}


