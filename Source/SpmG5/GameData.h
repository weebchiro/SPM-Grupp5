#pragma once

#include "CoreMinimal.h"
#include "GameData.generated.h"


USTRUCT(BlueprintType)
struct FGameData
{
	GENERATED_BODY()
	
public:
	
	
	
	/*UPROPERTY(BlueprintReadWrite)
	TMap<FString, int32> HighScores;*/
	
	UPROPERTY(BlueprintReadWrite)
	TMap<TSoftObjectPtr<UWorld>, int32> HighScores;

	UPROPERTY(BlueprintReadWrite)
	float MasterVolume = 100;
		
	UPROPERTY(BlueprintReadWrite)
	float MusicVolume = 100;
	
	UPROPERTY(BlueprintReadWrite)
	float SFXVolume = 100;
	
	UPROPERTY(BlueprintReadWrite)
	bool StopCameraMove = false;
	
};
