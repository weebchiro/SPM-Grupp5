// Marcus hopefully approves of this.

#include "SpawnAI.h"
#include "BoxSpawnRateManager.h"


void USpawnAI::SetupSpawner(int InAmountOfBoxesPerLevel, TMap<EBoxType, double> InSpawnRates, TMap<EBoxType, double> InDangerousTypes, TMap<EBoxAddress, double> InAddressTypes)
{
	Boxes.Empty();
	Addresses.Empty();
	
	TotalBoxCount = InAmountOfBoxesPerLevel;
	SpawnRates = InSpawnRates;
	DangerousTypes = InDangerousTypes;
	AddressTypes = InAddressTypes;
	
	ConvertAllPercentageToBoxes();
	
	/*UE_LOG(LogTemp, Warning, TEXT("All boxes remaining: %d"), TotalBoxCount);
	for (const FBoxSpawnInfo& Info : Boxes)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("BoxType: %s | RemainingBoxes: %d | CurrentSpawnRate: %.2f | CountSinceLastSpawn: %d"),
			*UEnum::GetValueAsString(Info.BoxType),
			Info.RemainingBoxes,
			Info.CurrentSpawnRate,
			Info.CountSinceLastSpawn);
	}
	
	for (const FAddressInfo& Info : Addresses)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("BoxType: %s | RemainingBoxes: %d | CurrentSpawnRate: %.2f | CountSinceLastSpawn: %d"),
			*UEnum::GetValueAsString(Info.BoxAddress),
			Info.RemainingBoxes,
			Info.CurrentSpawnRate,
			Info.CountSinceLastSpawn);
	}
	
	for (int i = 0; i < 40; i++)
	{
		UE_LOG(LogTemp, Warning, TEXT("Box %d: "), i);
		TArray<EBoxType> props = ConstructBox();
		EBoxAddress newThing = SetAddress();
		if (TotalBoxCount > 0) TotalBoxCount--;
		
		for (EBoxType Type : props)
		{
			UE_LOG(LogTemp, Warning, TEXT("		: %d"), Type);
		}
		
		UE_LOG(LogTemp, Warning, TEXT("		ADDRESS: %d"), newThing);
		
		for (const FBoxSpawnInfo& Info : Boxes)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("BoxType: %s | RemainingBoxes: %d | CurrentSpawnRate: %.2f | CountSinceLastSpawn: %d"),
				*UEnum::GetValueAsString(Info.BoxType),
				Info.RemainingBoxes,
				Info.CurrentSpawnRate,
				Info.CountSinceLastSpawn);
		}
		
		for (const FAddressInfo& Info : Addresses)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("BoxType: %s | RemainingBoxes: %d | CurrentSpawnRate: %.2f | CountSinceLastSpawn: %d"),
				*UEnum::GetValueAsString(Info.BoxAddress),
				Info.RemainingBoxes,
				Info.CurrentSpawnRate,
				Info.CountSinceLastSpawn);
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("All boxes remaining: %d"), TotalBoxCount);
	for (const FBoxSpawnInfo& Info : Boxes)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("BoxType: %s | RemainingBoxes: %d | CurrentSpawnRate: %.2f | CountSinceLastSpawn: %d"),
			*UEnum::GetValueAsString(Info.BoxType),
			Info.RemainingBoxes,
			Info.CurrentSpawnRate,
			Info.CountSinceLastSpawn);
	}
	
	for (const FAddressInfo& Info : Addresses)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("BoxType: %s | RemainingBoxes: %d | CurrentSpawnRate: %.2f | CountSinceLastSpawn: %d"),
			*UEnum::GetValueAsString(Info.BoxAddress),
			Info.RemainingBoxes,
			Info.CurrentSpawnRate,
			Info.CountSinceLastSpawn);
	}*/
}

TArray<EBoxType> USpawnAI::ConstructBox()
{
	for (FBoxSpawnInfo& Box : Boxes)
	{
		if (DangerousTypes.Contains(Box.BoxType))
		{
			SetUpSpawnRate(Box.CurrentSpawnRate, Box.RemainingBoxes, GetSpawnInfo(EBoxType::Dangerous).RemainingBoxes);
			continue;
		}
		if (Box.BoxType == EBoxType::Dangerous)
		{
			SetUpSpawnRate(Box.CurrentSpawnRate, Box.RemainingBoxes, GetSpawnInfo(EBoxType::Suspicious).RemainingBoxes);
			continue;
		}
		SetUpSpawnRate(Box.CurrentSpawnRate, Box.RemainingBoxes, TotalBoxCount);
	}
	
	TArray<EBoxType> Properties = DecideProperties();
	
	for (EBoxType BoxType : AllBoxTypes)
	{
		if (!Properties.Contains(BoxType)) GetSpawnInfo(BoxType).IncreaseCounter();
	}
	
	return Properties;
}

EBoxAddress USpawnAI::SetAddress()
{
	for (FAddressInfo& AddressInfo : Addresses)
	{
		SetUpSpawnRate(AddressInfo.CurrentSpawnRate, AddressInfo.RemainingBoxes, TotalBoxCount);
	}
	
	EBoxAddress Address = DecideAddress();
	
	for (EBoxAddress AddressType : AllAddressTypes)
	{
		if (Address != AddressType) GetAddressSpawnInfo(AddressType).IncreaseAddressCounter();
	}
	
	return Address;
}

TArray<EBoxType> USpawnAI::DecideProperties()
{
	TArray<EBoxType> Properties;
	if (Boxes.IsEmpty()) return Properties;
	
	//Roll if small or large
	if (!RollForProperty(EBoxType::Small))
	{
		if (GetSpawnInfo(EBoxType::Large).RemainingBoxes <= 0)
		{
			if (GetSpawnInfo(EBoxType::Small).RemainingBoxes > 0)
			{
				AddProperty(Properties, EBoxType::Small);
			}
			else
			{
				return Properties; // No boxes left
			}
		}
		else
		{
			AddProperty(Properties, EBoxType::Large);
		}
	}
	else
	{
		AddProperty(Properties, EBoxType::Small);
	}
	
	//Roll if fragile
	if (!GuaranteeProperty(Properties, EBoxType::Fragile, TotalBoxCount))
		if (RollForProperty(EBoxType::Fragile)) AddProperty(Properties, EBoxType::Fragile);
	
	//Roll if suspicious
	if (!GuaranteeProperty(Properties, EBoxType::Suspicious, TotalBoxCount))
	{
		if (!RollForProperty(EBoxType::Suspicious)) return Properties;
		AddProperty(Properties, EBoxType::Suspicious);
	}
	
	//Roll if dangerous
	if (!GuaranteeProperty(Properties, EBoxType::Dangerous, GetSpawnInfo(EBoxType::Suspicious).RemainingBoxes))
	{
		if (!RollForProperty(EBoxType::Dangerous)) return Properties;
		AddProperty(Properties, EBoxType::Dangerous);
	}
	
	double MaxSpawnRateDangerousBoxes = 0.0;
	
	for (FBoxSpawnInfo Box : Boxes)
	{
		if (DangerousTypes.Contains(Box.BoxType) && Box.RemainingBoxes > 0)
		{
			MaxSpawnRateDangerousBoxes += Box.CurrentSpawnRate;
		}
	}
	
	double BadBoxPercentage = GiveBadBoxesMaxPercentage(MaxSpawnRateDangerousBoxes);
	double CurrentRate = 0.0;
		
	for (FBoxSpawnInfo Box : Boxes)
	{
		if (DangerousTypes.Contains(Box.BoxType) && Box.RemainingBoxes > 0)
		{
			// Checks which bad box should be added
			if (BadBoxPercentage >= CurrentRate && BadBoxPercentage <= Box.CurrentSpawnRate + CurrentRate)
			{
				AddProperty(Properties, Box.BoxType);
				break;
			}
			
			CurrentRate += Box.CurrentSpawnRate;
		}
	}
	
	return Properties;
}

EBoxAddress USpawnAI::DecideAddress()
{
	EBoxAddress BoxAddress = EBoxAddress::TRIANGLE;
	
	if (Addresses.IsEmpty())
	{
		return BoxAddress;
	}
	
	double MaxSpawnRateAddress = 0.0;
	
	for (FAddressInfo Info : Addresses)
	{
		if (Info.RemainingBoxes > 0)
		{
			MaxSpawnRateAddress += Info.CurrentSpawnRate;
		}
	}
	
	double AddressPercentage = GiveBadBoxesMaxPercentage(MaxSpawnRateAddress);
	double CurrentRate = 0.0;
		
	for (FAddressInfo Info : Addresses)
	{
		if (Info.RemainingBoxes > 0)
		{
			// Checks which address should be added
			if (AddressPercentage >= CurrentRate && AddressPercentage <= Info.CurrentSpawnRate + CurrentRate)
			{
				BoxAddress = Info.BoxAddress;
				GetAddressSpawnInfo(BoxAddress).DecrementBoxAddressCount();
				break;
			}
			
			CurrentRate += Info.CurrentSpawnRate;
		}
	}
	
	return BoxAddress;
}

void USpawnAI::ConvertAllPercentageToBoxes()
{
	double TotalWeight = 0;
	
	for (const TPair<EBoxType, double>& Pair : DangerousTypes)
	{
		TotalWeight += Pair.Value;
	}
	
	// Set up all boxes of every enum
	for (EBoxType BoxType : AllBoxTypes)
	{
		if (BoxType == EBoxType::Large) continue;
		FBoxSpawnInfo BoxSpawnInfo = FBoxSpawnInfo(BoxType);
		int RemainingBoxes;
		//double SpawnRate = 0;
		
		if (DangerousTypes.Contains(BoxType))
		{
			double Percentage = ConvertWeightToPercentage(DangerousTypes[BoxType], TotalWeight);
			ConvertPercentageToBox(Percentage, RemainingBoxes, GetSpawnInfo(EBoxType::Dangerous).RemainingBoxes);
			//SetUpSpawnRate(SpawnRate, RemainingBoxes, GetSpawnInfo(EBoxType::Dangerous).RemainingBoxes);
		}
		else
		{
			BoxType == EBoxType::Dangerous ? ConvertPercentageToBox(SpawnRates[BoxType], RemainingBoxes, GetSpawnInfo(EBoxType::Suspicious).RemainingBoxes) : ConvertPercentageToBox(SpawnRates[BoxType], RemainingBoxes, TotalBoxCount);
			//BoxType == EBoxType::Dangerous ? SetUpSpawnRate(SpawnRate, RemainingBoxes, GetSpawnInfo(EBoxType::Suspicious).RemainingBoxes) : SetUpSpawnRate(SpawnRate, RemainingBoxes, TotalBoxCount); 
		}
		
		BoxSpawnInfo.RemainingBoxes = RemainingBoxes;
		//BoxSpawnInfo.CurrentSpawnRate = SpawnRate;
		Boxes.Add(BoxSpawnInfo);
		
		if (BoxType == EBoxType::Small)
		{
			FBoxSpawnInfo LargeSpawnInfo = FBoxSpawnInfo(EBoxType::Large);
			LargeSpawnInfo.RemainingBoxes = TotalBoxCount-RemainingBoxes;
			//SetUpSpawnRate(SpawnRate, LargeSpawnInfo.RemainingBoxes, TotalBoxCount);
			//LargeSpawnInfo.CurrentSpawnRate = SpawnRate;
			Boxes.Add(LargeSpawnInfo);
		}
	}
	
	TotalWeight = 0;
	
	for (const TPair<EBoxAddress, double>& Pair : AddressTypes)
	{
		TotalWeight += Pair.Value;
	}
	
	for (EBoxAddress Address : AllAddressTypes)
	{
		FAddressInfo BoxAddressInfo = FAddressInfo(Address);
		int RemainingBoxes;
		
		double Percentage = ConvertWeightToPercentage(AddressTypes[Address], TotalWeight);
		ConvertPercentageToBox(Percentage, RemainingBoxes, TotalBoxCount);
		
		BoxAddressInfo.RemainingBoxes = RemainingBoxes;
		Addresses.Add(BoxAddressInfo);
	}
	
	EnsurePercentageIsValid();
}

void USpawnAI::ConvertPercentageToBox(double Percentage, int& TypeOfRemainingBoxes, int DependencyFromAmount)
{
	Percentage = DependencyFromAmount * (Percentage / 100.f);
	Percentage = FMath::RoundHalfFromZero(Percentage);
	TypeOfRemainingBoxes = Percentage;
}

double USpawnAI::ConvertWeightToPercentage(double Weight, double TotalWeight)
{
	double Percentage = Weight / TotalWeight * 100.0;
	//Percentage = FMath::RoundHalfFromZero(Percentage);
	return Percentage;
}

void USpawnAI::EnsurePercentageIsValid()
{
	int TotalDangerousBoxes = 0;
	TArray<FBoxSpawnInfo> DangerousBoxes;
	
	for (FBoxSpawnInfo Box : Boxes)
	{
		if (DangerousTypes.Contains(Box.BoxType))
		{
			TotalDangerousBoxes+= Box.RemainingBoxes;
			DangerousBoxes.Add(Box);
		}
	}
	
	while (TotalDangerousBoxes > GetSpawnInfo(EBoxType::Dangerous).RemainingBoxes)
	{
		DangerousBoxes[FMath::RandRange(0, DangerousBoxes.Num()-1)].DecrementBoxCount();
	}
	
	int TotalAddressAmount = 0;
	TArray<FAddressInfo> RemainingAddresses;
	
	for (FAddressInfo Info : Addresses)
	{
		TotalAddressAmount += Info.RemainingBoxes;
		RemainingAddresses.Add(Info);
	}
	
	while (TotalAddressAmount > TotalBoxCount)
	{
		RemainingAddresses[FMath::RandRange(0, RemainingAddresses.Num()-1)].DecrementBoxAddressCount();
	}
}

void USpawnAI::SetUpSpawnRate(double& SpawnRate, int RemainingBoxType, int DependencyFromAmount)
{
	if (RemainingBoxType > 0)
	{
		SpawnRate = static_cast<double>(RemainingBoxType) / static_cast<double>(DependencyFromAmount);
		//UBoxSpawnRateManager->SpawnRate;
	}
	/*else
	{
		SpawnRate = 0.0;
	}*/
}

bool USpawnAI::RollForProperty(EBoxType BoxType)
{
	FBoxSpawnInfo SpawnInfo = GetSpawnInfo(BoxType);
	if (SpawnInfo.RemainingBoxes <= 0) return false;
	double Chance = GiveRandomPercentage();
	double SpawnRate = SpawnInfo.CurrentSpawnRate;
	
	if (Chance <= SpawnRate)
	{
		return true;
	}
	
	return false;
}

double USpawnAI::GiveRandomPercentage()
{
	return FMath::RandRange(0.0, 1.0);
}

double USpawnAI::GiveBadBoxesMaxPercentage(double MaxPercentage)
{
	return FMath::RandRange(0.0, MaxPercentage);
}

bool USpawnAI::GuaranteeProperty(TArray<EBoxType>& Properties, EBoxType BoxType, int DependencyFromAmount)
{
	if (GetSpawnInfo(BoxType).RemainingBoxes == DependencyFromAmount && GetSpawnInfo(BoxType).RemainingBoxes > 0)
	{
		AddProperty(Properties, BoxType);
		return true;
	}
	
	/*int CurrentCounter = 1/GetSpawnInfo(BoxType).CurrentSpawnRate;
	
	if (CurrentCounter < GetSpawnInfo(BoxType).CountSinceLastSpawn && GetSpawnInfo(BoxType).RemainingBoxes > 0)
	{
		AddProperty(Properties, BoxType);
		return true;
	}*/
	
	return false;
}

FBoxSpawnInfo& USpawnAI::GetSpawnInfo(EBoxType Type)
{
	for (FBoxSpawnInfo& Box : Boxes)
	{
		if (Box.BoxType == Type) return Box;
	}
	
	return Boxes[0];
}

FAddressInfo& USpawnAI::GetAddressSpawnInfo(EBoxAddress Type)
{
	for (FAddressInfo& Info : Addresses)
	{
		if (Info.BoxAddress == Type) return Info;
	}
	
	return Addresses[0];
}

void USpawnAI::AddProperty(TArray<EBoxType>& Properties, EBoxType Type)
{
	Properties.Add(Type);
	GetSpawnInfo(Type).DecrementBoxCount();
}
