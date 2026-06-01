// Marcus hopefully approves of this.

//#include "NoraWennerberg.h"
#include "SaveManager.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonTypes.h"
#include "Hal/FileManager.h"
#include "JsonObjectConverter.h"
#include "NiagaraDebuggerCommon.h"
#include "ScoreManager.h"
#include "SNegativeActionButton.h"



void USaveManager::SaveGame(const FGameData& GameData)
{
	TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(GameData);
	FString JsonString;
	
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	
	if (FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer))
	{
		FString FilePath = FPaths::ProjectSavedDir() / TEXT("SaveData.json");
	
		if (FFileHelper::SaveStringToFile(JsonString, *FilePath))
		{
			UE_LOG(LogTemp, Warning, TEXT("JSON FILE SAVED TO: %s"), *FilePath);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("JSON FILE FAILED: %s"), *FilePath);
		}
	}
	
}

FGameData USaveManager::LoadGame()
{
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("SaveData.json");
	FString JsonString;
	
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("JSON FILE FAILED TO LOAD: %s"), *FilePath);
		return FGameData();
	}
	
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		FGameData SaveData;
		if (FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &SaveData))
		{
			UE_LOG(LogTemp, Warning, TEXT("JSON SUCCESSFULLY LOADED"));
			return SaveData;
		}
	}
	
	UE_LOG(LogTemp, Error, TEXT("Something went wrong with reading json file"));
	return FGameData();
}

void USaveManager::ApplySettings()
{
}

bool USaveManager::DoesSaveExist()
{
	return true;
}
