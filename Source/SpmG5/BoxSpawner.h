#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.h"
#include "SpawnAI.h"
#include "BoxSpawner.generated.h"

UCLASS()
class SPMG5_API ABoxSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ABoxSpawner();
	
	//UFUNCTION(BlueprintImplementableEvent)
	AItem* SpawnBox();

protected:
	virtual void BeginPlay() override;

	AItem* SpawnItem();
	
	UPROPERTY(EditAnywhere)
	float SpawnRate = 1;
	
	FTimerHandle SpawnRateTimer;
	
	UPROPERTY(EditAnywhere)
	bool IsOldBoxSpawner = false;

	//UPROPERTY(EditAnywhere)
	//FVector SpawnLocation = FVector(50,50,120);

private:
	static int SpawnedBoxes;
	
	UPROPERTY()
	USpawnAI* SpawnAI;
	
	UFUNCTION(BlueprintCallable)
	AItem* SpawnItem(bool IsDangerous, bool IsLarge, bool IsFragile, bool IsSuspicious, EBoxAddress Address);
	
	void SpawnBoxOnPoint();
	void LoopSpawnBox(float NewSpawnRate);
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BoxMesh;

	UPROPERTY(EditAnywhere)
	bool SpawnOnPoint;
	
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* SpawnLocation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> BoxToSpawn;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> BombToSpawn;
	
	// UPROPERTY(EditAnywhere)
	// AActor* ItemToSpawn;

	// Not used currently
	UPROPERTY(EditAnywhere)
	float LargeBoxSpawnRate = 5;
	
	UPROPERTY(EditAnywhere)
	float FragileBoxSpawnRate = 10;
	
	UPROPERTY(EditAnywhere)
	float SuspiciousBoxSpawnRate = 50;
	
	UPROPERTY(EditAnywhere)
	float DangerousBoxSpawnRate = 50;
	
	static bool ShouldHappen(int percentage);
	
	EBoxAddress SetBoxAddress();
	
	//FULT!!!!! TA BORT EFTER SPELTEST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	UPROPERTY(EditAnywhere)
	bool PlayBoxSound = true;
};
