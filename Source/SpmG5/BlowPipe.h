// Marcus hopefully approves of this.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlowPipe.generated.h"

UCLASS()
class SPMG5_API ABlowPipe : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABlowPipe();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FTimerHandle IdleTimer;
	FTimerHandle BlowingTimer;
	FTimerHandle IndicatorTimer;
	// bool Blowing = false;
	float TimeBeforeBlowing = 0.0f;
	float TimeLeftBlowing = 0.0f;
	FRandomStream Stream = FRandomStream(0);
	
	UFUNCTION(BlueprintImplementableEvent, Blueprintable)
	void ActivateBlowing(bool Blowing);
	
	void Blow(float DeltaTime);
	void StartBlowing();
	void CallBlowMethod();
	void BlowStateLoop(float LoopRate);
	void ShowIndicator();
	UFUNCTION(BlueprintCallable)
	float GetForceMultiplier(){return ForceMultiplier;}
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//UPROPERTY(EditAnywhere)
	FVector BlowBoxSize = FVector(120.0f, 50.0f, 50.0f);

	//UPROPERTY(EditAnywhere)
	FVector Offcset = FVector(0.0f, 100.0f, 0.0f);
	
	UPROPERTY(EditAnywhere, Category="Blow Settings")
	float BlowTime = 5;
	
	UPROPERTY(EditAnywhere, Category="Blow Settings")
	float BlowIndicatorTime = 3;
	
	UPROPERTY(EditAnywhere, Category="Blow Settings")
	FVector2f WaitTimeRange = FVector2f(5.0,10.0);
	
	UPROPERTY(EditAnywhere,Category="Blow Settings")
	float ForceMultiplier = 500;
	
	UPROPERTY(VisibleAnywhere)
	USceneComponent* SceneComponent;
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BoxMesh;
	
};
