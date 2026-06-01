// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
//#include "BoxDestroyer.h"
#include "AudioEnums.h"
#include "FMODBlueprintStatics.h"
#include "NiagaraSystem.h"
#include "Item.generated.h"

class AConveyorBeltUpgraded;
class AConveyorBelt; // forward-declaration

UENUM(BlueprintType)
enum class EBoxAddress : uint8
{
	CIRCLE,
	SQUARE,
	TRIANGLE,
};

USTRUCT(BlueprintType)
struct FAddressInfo
{
	GENERATED_BODY()
	
	FAddressInfo(){}
	
	FAddressInfo(EBoxAddress InAddress){BoxAddress = InAddress;}
	
	EBoxAddress BoxAddress;
	int RemainingBoxes;
	double CurrentSpawnRate;
	int CountSinceLastSpawn = 0;
	
	void DecrementBoxAddressCount()
	{
		RemainingBoxes--;
		CountSinceLastSpawn = 0;
	}
	
	void IncreaseAddressCounter()
	{
		CountSinceLastSpawn++;
	}
};

UCLASS()
class AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();
	
	UPROPERTY(VisibleAnywhere)
	AConveyorBelt* Conveyor = nullptr; //pekar til conveyor om den är attached
	UPROPERTY(VisibleAnywhere)
	AConveyorBeltUpgraded* ConveyorUpgraded = nullptr; //pekar til conveyor om den är attached

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsInStation = false;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	int CollisionSoundsPlayed;
	
	UPROPERTY(EditAnywhere, Category="Audio")
	EAudioItemType AudioType;
	
	UPROPERTY(EditAnywhere, Category="Audio")
	UFMODEvent* CollisionSFX;
	
	UPROPERTY(EditAnywhere, Category="Audio")
	float CollisionSFXCooldown = 0.1;
	float CollisionSFXTimer;
	
	UFMODAudioComponent* CollisionSFXInstance;
	
	UPROPERTY(EditAnywhere, Category="Audio")
	float SFXNormalImpulseMin = 50.0f;
	
	UPROPERTY(EditAnywhere, Category="Audio")
	float SFXNormalImpulseMax = 1000.0f;
	
	UPROPERTY(EditAnywhere, Category="Audio")
	float SFXCollisionVolumeMin = 0.0f;
	
	UPROPERTY(EditAnywhere, Category="Audio")
	float SFXCollisionVolumeMax = 1.0f;
	
	UPROPERTY(EditAnywhere, Category="Audio")
	UFMODEvent* DestroySFX;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBoxAddress Address = EBoxAddress::SQUARE;
	
	UPROPERTY(EditAnywhere)
	bool IsFragile;

	UPROPERTY(EditAnywhere)
	bool IsLarge;
	
	UPROPERTY(EditAnywhere)
	bool IsSuspicious;
	
	UPROPERTY(EditAnywhere)
	bool IsDangerous;
	
	UPROPERTY(EditAnywhere)
	bool IsScanned = false;
	
	UPROPERTY(EditAnywhere)
	bool IsInspected = false;
	
	bool IsHeld = false;
	
	bool HasBeenDroppedOff = false;
	
	UPROPERTY(EditAnywhere)
	int SmallBoxPoints = 5;
	
	UPROPERTY(EditAnywhere)
	int LargeBoxPoints = 10;
	
	UPROPERTY(EditAnywhere)
	int FragileBoxPoints = 5;
	
	UPROPERTY(EditAnywhere)
	int ScannedBoxPoints = 5;
	
	UPROPERTY(EditAnywhere)
	int InspectedBoxPoints = 5;
	
	UPROPERTY(EditAnywhere)
	int WrongBoxPoints = -15;
	
	int Points;
	
	UPrimitiveComponent* PrimComp;
	
	UPROPERTY(BlueprintReadOnly)
	AActor* MostRecentHolder;
	
	void CalculateIfBreakIfFragile();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable)
	void SetPoints();
	
	UFUNCTION(BlueprintCallable)
	void SetNegativePoints();
	
	UFUNCTION(BlueprintCallable)
	UPrimitiveComponent* GetPrimitive(){return PrimComp;}
	
	void SetPhysics(bool SetTo);
	void SetCollitionDefultProfile(bool SetTo);
	void ResetVelocity(){PrimComp->SetPhysicsLinearVelocity(FVector(0,0,0));}
	void AddVelocity(int Force, FVector Direction){PrimComp->SetPhysicsLinearVelocity(Direction * Force + GetActorUpVector() * Force/2);}
	
	UFUNCTION(BlueprintCallable)
	virtual void Disintegrate(bool bThrownInTrash);
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
			   FVector NormalImpulse,
			   const FHitResult& Hit);
	
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//USceneComponent* BaseStaticMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* BaseMesh;
	
	void AddImpulse(FVector Point, float Strength);
	
	UPROPERTY(EditAnywhere)
	int MaxSpeedIfFragile = 400;
	
	UPROPERTY(EditAnywhere)
	bool ShouldBreakOnImpact = false;
	
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* FragileBreakParticles;
	
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* TrashBreakParticles;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetAudioType();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetIsFragile(){return IsFragile;}
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetIsLarge(){return IsLarge;}
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetIsSuspicious(){return IsSuspicious;}
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetIsDangerous(){return IsDangerous;}
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetIsScanned(){return IsScanned;}
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetIsInspected(){return IsInspected;}
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetPoints(){return Points;}
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	EBoxAddress GetAddress(){return Address;}
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetIsHeld(){return IsHeld;}
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetHasBeenDroppedOff(){return HasBeenDroppedOff;}
	
	UFUNCTION(BlueprintCallable)
	void SetMostRecentHolder(AActor* holder);
	
	void SetIsLarge(bool SetTo);
	void SetIsFragile(bool SetTo);
	void SetIsSuspicious(bool SetTo);
	void SetIsDangerous(bool SetTo);
	void SetAddress(EBoxAddress NewAddress);

	UFUNCTION(BlueprintImplementableEvent, Blueprintable, BlueprintCallable)
	void ActivateOvelay(bool SetTo);
	
	
	virtual void CallActivateOvelay(bool SetTo) {ActivateOvelay(SetTo);}
	
	UFUNCTION(BlueprintCallable)
	void SetIsScanned(bool SetTo);
	
	UFUNCTION(BlueprintCallable)
	void SetIsInspected(bool SetTo);
	
	UFUNCTION(BlueprintCallable)
	void SetIsHeld(bool SetTo);
	
	UFUNCTION(BlueprintCallable)
	void SetHasBeenDroppedOff(bool SetTo);
		
	//FULT!!!!! TA BORT EFTER SPELTEST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
	UPROPERTY(EditAnywhere, blueprintReadWrite)
	bool PlaySound = true;
	
	void SetPlaySound(bool SetTo){PlaySound = SetTo;}
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetAddressDecal();
};
