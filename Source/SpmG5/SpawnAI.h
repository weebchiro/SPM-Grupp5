// Marcus hopefully approves of this.

#pragma once

#include "Item.h"
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SpawnAI.generated.h"

UENUM(BlueprintType)
enum class EBoxType : uint8
{
	Small,
	Large,
	Fragile,
	Suspicious,
	Dangerous,
	Bomb,
	ToxicWaste,
	FlashBang
};

USTRUCT(BlueprintType)
struct FBoxSpawnInfo
{
	GENERATED_BODY()
	
	FBoxSpawnInfo(){}
	
	FBoxSpawnInfo(EBoxType Type){BoxType = Type;}
	
	EBoxType BoxType;
	int RemainingBoxes;
	double CurrentSpawnRate;
	int CountSinceLastSpawn = 0;
	
	void DecrementBoxCount()
	{
		RemainingBoxes--;
		CountSinceLastSpawn = 0;
	}
	
	void IncreaseCounter()
	{
		CountSinceLastSpawn++;
	}
};

UCLASS()
class SPMG5_API USpawnAI : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetupSpawner(int InAmountOfBoxesPerLevel, TMap<EBoxType, double> InSpawnRates, TMap<EBoxType, double> InDangerousTypes, TMap<EBoxAddress, double> InAddressTypes);

	TArray<EBoxType> ConstructBox();
	EBoxAddress SetAddress();

private:
	int TotalBoxCount;
	TMap<EBoxType, double> SpawnRates;
	TMap<EBoxType, double> DangerousTypes;
	TMap<EBoxAddress, double> AddressTypes;
	
	TArray<FBoxSpawnInfo> Boxes; //TMap<EBoxType, FBoxSpawnInfo> SpawnInfos; POTENTIELL UPGRADE ???
	TArray<EBoxType> AllBoxTypes{EBoxType::Small, EBoxType::Large, EBoxType::Fragile, EBoxType::Suspicious, EBoxType::Dangerous, EBoxType::Bomb, EBoxType::ToxicWaste, EBoxType::FlashBang};
	
	TArray<FAddressInfo> Addresses;
	TArray<EBoxAddress> AllAddressTypes{EBoxAddress::CIRCLE, EBoxAddress::SQUARE, EBoxAddress::TRIANGLE};
	
	FBoxSpawnInfo& GetSpawnInfo(EBoxType Type);
	FAddressInfo& GetAddressSpawnInfo(EBoxAddress Type);
	
	TArray<EBoxType> DecideProperties();
	EBoxAddress DecideAddress();
	
	void ConvertAllPercentageToBoxes();
	void ConvertPercentageToBox(double Percentage, int& TypeOfRemainingBoxes, int DependencyFromAmount);
	double ConvertWeightToPercentage(double Weight, double TotalWeight);
	void EnsurePercentageIsValid();
	void SetUpSpawnRate(double& SpawnRate, int RemainingBoxType, int DependencyFromAmount);
	bool RollForProperty(EBoxType BoxType);
	double GiveRandomPercentage();
	double GiveBadBoxesMaxPercentage(double MaxPercentage);
	bool GuaranteeProperty(TArray<EBoxType>& Properties, EBoxType BoxType, int DependencyFromAmount);
	void AddProperty(TArray<EBoxType>& Properties, EBoxType Type);
};
