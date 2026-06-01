// Marcus hopefully approves of this.

#pragma once

#include "CoreMinimal.h"
#include "SpawnAI.h"
#include "Blueprint/UserWidget.h"
#include "LevelButton.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FLevelInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Default")
	TSoftObjectPtr<UWorld> Level;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Default")
	int PointsToPass;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Default")
	int ShiftTime;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Default")
	FText LevelName;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Default")
	bool IsTutorial;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Default")
	int AmountOfBoxesPerLevel = 100.f;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Default", EditFixedSize, meta=(UIMin = "0.0", UIMax = "100.0"), meta=(ReadOnlyKeys), meta=(ShowOnlyInnerProperties))
	TMap<EBoxType, double> SpawnRates = {{ EBoxType::Small, 75.0 }, { EBoxType::Fragile, 15.0 },{ EBoxType::Suspicious, 40.0 },{ EBoxType::Dangerous, 50.0 }};
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Default", EditFixedSize, meta=(ReadOnlyKeys), meta=(ShowOnlyInnerProperties))
	TMap<EBoxType, double> DangerousTypes = {{ EBoxType::Bomb, 1.0 }, { EBoxType::ToxicWaste, 0.0 }, { EBoxType::FlashBang, 0.0 }};
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Default", EditFixedSize, meta=(ReadOnlyKeys), meta=(ShowOnlyInnerProperties))
	TMap<EBoxAddress, double> BoxAddresses = {{ EBoxAddress::CIRCLE, 1.0 }, { EBoxAddress::SQUARE, 1.0 }, { EBoxAddress::TRIANGLE, 1.0 }};
};

UCLASS()
class SPMG5_API ULevelButton : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Default", meta=(ShowOnlyInnerProperties))
	FLevelInfo LevelInfo;
};
