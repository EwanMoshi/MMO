// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "UsableActor.generated.h"

UCLASS()
class MMO_API AUsableActor : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UStaticMeshComponent* MeshComp;

public:	
	
	/* Player begins looking at object */
	virtual void OnBeginFocus();

	/* Player stops looking at object */
	virtual void OnEndFocus();

	/* When player interacts with object */
	virtual void OnUsed(APawn* InstigatorPawn);	
};
