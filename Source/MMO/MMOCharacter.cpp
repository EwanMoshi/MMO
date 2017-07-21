// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "MMO.h"
#include "MMOCharacter.h"
#include "MMOProjectile.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"
#include "UsableActor.h"
#include "MMOPlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AMMOCharacter

AMMOCharacter::AMMOCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// tick every frame
	PrimaryActorTick.bCanEverTick = true;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(0, 0, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 30.0f, 10.0f);

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	//Mesh1P->AttachParent = FirstPersonCameraComponent;
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->RelativeLocation = FVector(0.f, 0.f, -150.f);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;


	maxUseDistance = 800;

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P are set in the
	// derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMMOCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	
	//InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AMMOCharacter::TouchStarted);
	if( EnableTouchscreenMovement(InputComponent) == false )
	{
		InputComponent->BindAction("Fire", IE_Pressed, this, &AMMOCharacter::OnMouseLeftPressed);
		InputComponent->BindAction("Fire", IE_Released, this, &AMMOCharacter::OnMouseLeftRelease);
	}
	
	InputComponent->BindAxis("MoveForward", this, &AMMOCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AMMOCharacter::MoveRight);
	
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &AMMOCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AMMOCharacter::LookUpAtRate);

	// Weapons
	InputComponent->BindAction("Targeting", IE_Pressed, this, &AMMOCharacter::OnStartTargeting);
	InputComponent->BindAction("Targeting", IE_Released, this, &AMMOCharacter::OnEndTargeting);

	// Interaction
	InputComponent->BindAction("Use", IE_Pressed, this, &AMMOCharacter::Use);
}


void AMMOCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

}

void AMMOCharacter::OnFire()
{ 
	// UE_LOG(LogTemp, Log, TEXT("PRINT")); // Can print to output log 

	// try and fire a projectile
	if (ProjectileClass != NULL) {
		GetWorldTimerManager().SetTimer(fireHandle, this, &AMMOCharacter::FireWeapon, 0.1f, true);
		GetWorldTimerManager().SetTimer(recoilHandle, this, &AMMOCharacter::RecoilWeapon, 0.1f, true);
		FireWeapon();
		RecoilWeapon();
		//ResetRecoil();
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		//UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if(FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if(AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}

}


void AMMOCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMMOCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMMOCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMMOCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}



void AMMOCharacter::FireWeapon() {
	// Get the camera transform
	FVector CameraLoc;
	FRotator CameraRot;
	GetActorEyesViewPoint(CameraLoc, CameraRot);
	// MuzzleOffset is in camera space, so transform it to world space before offsetting from the camera to find the final muzzle position
	FVector const MuzzleLocation = CameraLoc + FTransform(CameraRot).TransformVector(GunOffset);
	FRotator MuzzleRotation = CameraRot;
	//MuzzleRotation.Pitch += 10.0f;          // skew the aim upwards a bit
	RecoilRecovery = MuzzleRotation.Pitch; //store where to recover ::ADDED IN
	UWorld* const World = GetWorld();
	if (World)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = Instigator;
		// spawn the projectile at the muzzle
		AMMOProjectile* const Projectile = World->SpawnActor<AMMOProjectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, SpawnParams);
		if (Projectile)
		{
			// find launch direction
			FVector const LaunchDir = MuzzleRotation.Vector();
			Projectile->InitVelocity(LaunchDir);
		}
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL) {
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AMMOCharacter::RecoilWeapon() {
	FinalRecoilPitch = Recoil * FMath::FRandRange(-1.0f, -1.25f);
	float FinalRecoilYaw = Recoil * FMath::FRandRange(0.2f, -0.2f); //side ways recoil
	AddControllerPitchInput(FinalRecoilPitch);
	AddControllerYawInput(FinalRecoilYaw);
	//isRecoiling = true;
}

void AMMOCharacter::ResetRecoil() {

	isRecoiling = false;
	// UE_LOG(LogTemp, Log, TEXT("PRINT")); // Can print to output log 

	//Recoil = FMath::FInterpTo(Recoil, 0, 0.1f, 10.0f);
	 //RecoilRecovery = FMath::FInterpTo(RecoilRecovery, -Recoil, 0.1f, 20.0f);

	 //AddControllerPitchInput(Recoil + RecoilRecovery);

}

void AMMOCharacter::OnMouseLeftPressed()
{
	OnFire();
}

void AMMOCharacter::OnMouseLeftRelease()
{
	GetWorldTimerManager().ClearTimer(fireHandle);
	GetWorldTimerManager().ClearTimer(recoilHandle);
}

/*
Perform ray-trace and return closest UsableActor
*/

AUsableActor* AMMOCharacter::GetUsableActorInView() {
	FVector camLoc;
	FRotator camRot;

	if (Controller == NULL) {
		return NULL;
	}

	Controller->GetPlayerViewPoint(camLoc, camRot);
	const FVector traceStart = camLoc;
	const FVector direction = camRot.Vector();
	const FVector traceEnd = traceStart + (direction * maxUseDistance);

	FCollisionQueryParams traceParams(FName(TEXT("TraceUsableActor")), true, this);
	traceParams.bTraceAsyncScene = true;
	traceParams.bReturnPhysicalMaterial = false;
	traceParams.bTraceComplex = true;

	FHitResult Hit(ForceInit);
	//GetWorld()->LineTraceSingle(Hit, traceStart, traceEnd, ECC_Visibility, traceParams);

	return Cast<AUsableActor>(Hit.GetActor());
}

void AMMOCharacter::Use() {
	// If Use called on client, send request to server - only allow use function to be called on server
	if (Role == ROLE_Authority) {
		AUsableActor* usable = GetUsableActorInView();

		if (usable) {
			usable->OnUsed(this);
		}
	}
	else {
		//ServerUse();
	}
}

void AMMOCharacter::OnStartTargeting() {
	SetTargeting(true);
}


void AMMOCharacter::OnEndTargeting() {
	SetTargeting(false);
}

void AMMOCharacter::SetTargeting(bool newTargeting)
{
	bIsTargeting = newTargeting;

	if (Role < ROLE_Authority)
	{
		//ServerSetTargeting(newTargeting);
	}
}

bool AMMOCharacter::IsTargeting() const {
	return bIsTargeting;
}

/** Create baseCharacterClass and place these functions there
void AMMOCharacter::ServerSetTargeting_Implementation(bool newTargeting) {
	SetTargeting(newTargeting);
}


bool AMMOCharacter::ServerSetTargeting_Validate(bool newTargeting) {
	return true;
}*/










void AMMOCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AMMOCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = false;
}

void AMMOCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
	{
		if (TouchItem.bIsPressed)
		{
			if (GetWorld() != nullptr)
			{
				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
				if (ViewportClient != nullptr)
				{
					FVector MoveDelta = Location - TouchItem.Location;
					FVector2D ScreenSize;
					ViewportClient->GetViewportSize(ScreenSize);
					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
					if (ScaledDelta.X != 0.0f)
					{
						TouchItem.bMoved = true;
						float Value = ScaledDelta.X * BaseTurnRate;
						AddControllerYawInput(Value);
					}
					if (ScaledDelta.Y != 0.0f)
					{
						TouchItem.bMoved = true;
						float Value = ScaledDelta.Y* BaseTurnRate;
						AddControllerPitchInput(Value);
					}
					TouchItem.Location = Location;
				}
				TouchItem.Location = Location;
			}
		}
	}
}



bool AMMOCharacter::EnableTouchscreenMovement(class UInputComponent* InputComponent)
{
	bool bResult = false;
	if (FPlatformMisc::GetUseVirtualJoysticks() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		bResult = true;
		InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AMMOCharacter::BeginTouch);
		InputComponent->BindTouch(EInputEvent::IE_Released, this, &AMMOCharacter::EndTouch);
		InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AMMOCharacter::TouchUpdate);
	}
	return bResult;
}