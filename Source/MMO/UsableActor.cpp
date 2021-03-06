// Fill out your copyright notice in the Description page of Project Settings.

#include "MMO.h"
#include "UsableActor.h"


AUsableActor::AUsableActor(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MeshComp = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("MESH"));
	RootComponent = MeshComp;
}

void AUsableActor::OnUsed(APawn* InstigatorPawn) {
}

void AUsableActor::OnBeginFocus() {
	// Used by custom PostProcess to render outlines
	MeshComp->SetRenderCustomDepth(true);
}

void AUsableActor::OnEndFocus() {
	// Used by custom PostProcess to render outlines
	MeshComp->SetRenderCustomDepth(false);
}