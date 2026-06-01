// Marcus hopefully approves of this.

#pragma once

#include "CoreMinimal.h"
#include "GameData.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SaveManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadSave);

struct FGameData;

UCLASS()
class SPMG5_API USaveManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
		
	UFUNCTION(BlueprintCallable)
	static void SaveGame(const FGameData& GameData);
	
	UFUNCTION(BlueprintCallable)
	static FGameData LoadGame();
	
	UFUNCTION(BlueprintCallable)
	static void ApplySettings();
	
	UFUNCTION(BlueprintCallable)
	static bool DoesSaveExist();
	
private:
	static FString GetSavePath();

};
