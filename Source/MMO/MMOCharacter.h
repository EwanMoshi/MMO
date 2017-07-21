// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "MMOCharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class AMMOCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;
public:
	AMMOCharacter();

	// This function is called every frame
	virtual void Tick(float DeltaSeconds) override;

	/************************************************************************/
	/* Interaction                                                          */
	/************************************************************************/

	class AUsableActor* GetUsableActorInView();

	/*Max distance to interact with usable actors. */
	UPROPERTY(EditDefaultsOnly, Category = "ObjectInteraction")
		float maxUseDistance;


	/* Use the usable actor currently in focus (if any) */
	virtual void Use();

	//UFUNCTION(Server, Reliable, WithValidation)
		//void ServerUse();

	/************************************************************************/
	/* Attributes                                                           */
	/************************************************************************/


	//UFUNCTION(BlueprintCallable, Category = "PlayerCondition")
		//float GetMaxHealth() const;

	/************************************************************************/
	/* Targeting                                                            */
	/************************************************************************/

	void OnStartTargeting();

	void OnEndTargeting();

	void SetTargeting(bool NewTargeting);

	//UFUNCTION(Server, Reliable, WithValidation)
		//void ServerSetTargeting(bool NewTargeting);

	/* Is player aiming down sights */
	UFUNCTION(BlueprintCallable, Category = "Targeting")
		bool IsTargeting() const;

	//UPROPERTY(Transient, Replicated)
		bool bIsTargeting;


	/************************************************************************/
	/* Misc                                                                 */	
	/************************************************************************/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Recoil) // maybe remove this UPROPERTY
	float FinalRecoilPitch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Recoil)
	float Recoil;
	float RecoilRecovery; // Stores current positive (downwards pitch rotation) recoil recovery

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AMMOProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

protected:
	
	/** Checking for Fire Weapon. */
	void OnFire();

	/** Fire the weapon */
	void FireWeapon();

	/** Recoil the weapon */
	void RecoilWeapon();

	/** Recoil the weapon */
	void ResetRecoil();

	/** Left Mouse Released */
	void OnMouseLeftRelease();

	/** Left Mouse Pressed */
	void OnMouseLeftPressed();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }


private:
	FTimerHandle fireHandle;
	FTimerHandle recoilHandle;

	//UPROPERTY(EditDefaultsOnly, Category = "PlayerCondition", Replicated)
		float Health;

		bool isRecoiling;
};

