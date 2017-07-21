// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/PlayerCameraManager.h"
#include "MMOPlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class MMO_API AMMOPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_UCLASS_BODY()
	
	/* Update FOV */
	virtual void UpdateCamera(float DeltaTime) override;
	
public:

	/* hip fire FOV (default)*/
	float normalFOV;

	/* down sight FOV */
	float targetingFOV;
};
