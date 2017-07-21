// Fill out your copyright notice in the Description page of Project Settings.

#include "MMO.h"
#include "MMOPlayerCameraManager.h"
#include "MMOCharacter.h"
#include "GameFramework/InputSettings.h"


AMMOPlayerCameraManager::AMMOPlayerCameraManager(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	normalFOV = 90.0f;
	targetingFOV = 65.0f;

	ViewPitchMin = -80.0f;
	ViewPitchMax = 87.0f;
	bAlwaysApplyModifiers = true;
}


void AMMOPlayerCameraManager::UpdateCamera(float DeltaTime)
{
	AMMOCharacter* MyPawn = PCOwner ? Cast<AMMOCharacter>(PCOwner->GetPawn()) : NULL;
	if (MyPawn)
	{
		const float TargetFOV = MyPawn->IsTargeting() ? targetingFOV : normalFOV;
		DefaultFOV = FMath::FInterpTo(DefaultFOV, TargetFOV, DeltaTime, 20.0f);
		SetFOV(DefaultFOV);
	}


	Super::UpdateCamera(DeltaTime);
}


