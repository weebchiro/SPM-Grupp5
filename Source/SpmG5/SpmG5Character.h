// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/BoxComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Item.h"
#include "Logging/LogMacros.h"
#include "FMODEvent.h"
#include "GameManager.h"
#include "SpmG5Character.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(abstract)
class ASpmG5Character : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	/*
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera #1#
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	*/
	
protected:
	
	FTimerHandle HoldingTimer;
	
	UPROPERTY()
	UPrimitiveComponent* PrimComp;	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USceneComponent* HoldingLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IncapacitationMeter = 0; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncreaseIncapacitation = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxIncapacitation = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRagdolling = false;
	
	UPROPERTY(EditAnywhere)
	FVector PickUpBoxSize = FVector(80.0f, 50.0f, 120.0f);
	
	UPROPERTY(EditAnywhere)
	float Offset = 10.0f;

	UPROPERTY(EditAnywhere)
	float RotateSpeedMult = 3.0f;

	UPROPERTY(EditAnywhere)
	float TurningSpeed = 5;
	
	UPROPERTY(BlueprintReadWrite, Category="Trowing settings")
	bool Throwing = false;
	UPROPERTY(BlueprintReadWrite, Category="Trowing settings")
	bool Thrown = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trowing settings")
	float MaxThrowForce = 500.0f;
	UPROPERTY(EditAnywhere, Category="Trowing settings")
	float ThrowForceIncrease = 500.0f;
	UPROPERTY(EditAnywhere, Category="Trowing settings")
	float StartingThrowForce = 300.0f;
	UPROPERTY(BlueprintReadOnly, Category="Trowing settings")
	float CurrentThrowForce = 300.0f;
	UPROPERTY(EditAnywhere, Category="Trowing settings")
	float MovementDebufMult = 8.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool HasItem = false;
	
	UPROPERTY(EditAnywhere)
	float StickDeadZone = 0.2f;

	//FHitResult HitResultBox;
	//FHitResult HitResultConvayer;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AItem* HeldItem = nullptr;
	UPROPERTY()
	AItem* ItemToPickup;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* PickupOrDropAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ThrowAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* PauseAction;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool IsInteracting = false;
	
	/* SFX */
	UPROPERTY(EditAnywhere, Category="Audio")
	UFMODEvent* PickupSFX;
	
	UPROPERTY(EditAnywhere, Category="Audio")
	UFMODEvent* DropSFX;
	
	UPROPERTY(EditAnywhere, Category="Audio")
	UFMODEvent* ThrowSFX;

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateThrowBar();
	UFUNCTION(BlueprintImplementableEvent)
	void ShowOrHideThrowBar(bool SetTo);
	
public:
	/** Constructor */
	ASpmG5Character();	

protected:
	//Om ska implementera som timer istället
	// FTimerHandle OutlineUpdateTimer;
	// float OutlineUpdateRate = 0.01f;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Tick(float DeltaTime) override;
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	//metod som sweepar för att hitta items & interactable
	TArray<FHitResult> DoSweep();
	void FindBoxToPickup();
	void ChooseInteractOrPickup();
	
	void AttachPackage();
	
	
	void Hold();
	
	UFUNCTION(BlueprintCallable)
	void Pickup(AItem* Item);
	
	void Throw(const FInputActionValue& Value);
	void ChargeUpThrow(const FInputActionValue& Value);
	FRotator Rotate(FVector2d Input);
	

public:
	void AddVelocity(FVector Force){AddMovementInput(Force);}

	
	UFUNCTION(BlueprintCallable, Blueprintable, BlueprintImplementableEvent)
	void AttatchPackageToConstaint(AItem* ItemToAttatch);
	UFUNCTION(BlueprintCallable, Blueprintable, BlueprintImplementableEvent)
	void DeAattatchPackageToConstaint(AItem* ItemToDeattatch);
	
	UFUNCTION(BlueprintCallable, Category="Item Reactions")
	void IncreaseIncapacitation(float Increase = 2);
	UFUNCTION(BlueprintCallable, Category="Item Reactions")
	void DecreaseIncapacitation(float Decrease = 1);

	UFUNCTION(BlueprintImplementableEvent, Blueprintable, BlueprintCallable, Category="Physics")
	void DoRagdoll();
	UFUNCTION(BlueprintImplementableEvent, Blueprintable, BlueprintCallable, Category="Physics")
	void StopRagdoll();
	
	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Input")
	void Interact(const UObject* Station);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Input")
	bool GetHasItem();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Input")
	AItem* GetItem();
	
	UFUNCTION(BlueprintCallable, Category="Input")
	AItem* Drop();
	
	UFUNCTION(BlueprintCallable, Category="Input")
	void Pause();
};

