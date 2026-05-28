// Marcus hopefully approves of this.


#include "ScoreManager.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "GameManager.h"

int UScoreManager::GetScore()
{
	return Score;
}

int UScoreManager::GetAddedScore()
{
	return AddedScore;
}

int UScoreManager::GetComboMultiplier()
{
	return ComboMultiplier;
}

float UScoreManager::GetComboTimeRate()
{
	return ComboTimerMAX / TimeRate;
	//if (ComboTimeRemaining <= 0)
		//return 0;
	//return ComboTimeRemaining/ComboTimerMAX;
}

void UScoreManager::StartTimer()
{
	//OnTimeRunsOut.RemoveAll(this);
	GetWorld()->GetTimerManager().ClearTimer(ComboTimer);
	ComboTimeRemaining = ComboTimerMAX;
	
	//CurrentMin = ShiftLengthInSeconds / 60;
	//CurrentSec = ShiftLengthInSeconds % 60;
	
	GetWorld()->GetTimerManager().SetTimer(
		ComboTimer, this, &UScoreManager::CountdownCombo, TimeRate, true);
}

void UScoreManager::CountdownCombo()
{
	ComboTimeRemaining -= TimeRate;

	// Update timer on screen
	
	if (ComboTimeRemaining <= 0)
	{
		//GetWorld()->GetTimerManager().PauseTimer(ShiftTimer);
		GetWorld()->GetTimerManager().ClearTimer(ComboTimer);
		//OnTimeRunsOut.Broadcast();
		ChangeCombo(-1);
	}

	//CurrentMin = ComboTimeRemaining / 60;
	//CurrentSec = ComboTimeRemaining % 60;
	
	
}

void UScoreManager::ChangeCombo(int Change)
{
	if (Change == 999) //SENTINEL FÖR RESET
	{
		ComboMultiplier = 1;
		GetWorld()->GetTimerManager().ClearTimer(ComboTimer);
		ComboTimeRemaining = 0;
		//OnComboChanged.Broadcast();
		OnComboBreak.Broadcast();
		return;
	}
	
	ComboMultiplier+= Change;
	if (ComboMultiplier < 1)
	{
		ComboMultiplier = 1;
		OnComboBreak.Broadcast();
		return;
	}

	StartTimer();
	if (ComboMultiplier > 5) //max 5 i kombo multi
	{
		ComboMultiplier = 5;
	}
	OnComboChanged.Broadcast();
}

int UScoreManager::GetCorrectlySortedBoxes()
{
	return CorrectlySortedBoxes;
}

void UScoreManager::AddScore(int ScoreChange)
{
	AddedScore = ScoreChange;
	int NewScore = Score + ScoreChange;
	
	//Höj combo multiplier vid rätt sortering, och ta bort vid fel sortering
	if (AddedScore < 0)
	{
		if (bUseComboMultiplier)
		{
			ChangeCombo(999);
		}
			
	}
	else
	{
		
		if (bUseComboMultiplier)
		{
			NewScore = Score + (ScoreChange * ComboMultiplier);
			ChangeCombo(1);
		}
		++CorrectlySortedBoxes;
	}
	
	if (NewScore < 0)
	{
		NewScore = 0;
		OnScoreChanged.Broadcast();
	}
	
	//om det skett en visible förändring
	if (NewScore != Score)
	{
		Score = NewScore;
		OnScoreChanged.Broadcast();
	}
	
 	UE_LOG(LogTemp, Warning, TEXT("Score: %d"), Score);
}


void UScoreManager::ResetScore()
{
	Score = 0.f;
}

void UScoreManager::SaveScore()
{
	TSoftObjectPtr<UWorld> CurrentLevel = Cast<UGameManager>(GetWorld()->GetGameInstance())->CurrentLevel;
	
	if (Score > ScoreMap.FindOrAdd(CurrentLevel))
	{
		ScoreMap[CurrentLevel] = Score;
	}
	ResetScore();
}


