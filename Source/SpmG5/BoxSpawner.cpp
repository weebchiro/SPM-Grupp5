#include "BoxSpawner.h"

#include "Bomb.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ABoxSpawner::ABoxSpawner()
{
 	PrimaryActorTick.bCanEverTick = true;
	
	BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
	SetRootComponent(BoxMesh);

	SpawnLocation = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnLocation"));
	SpawnLocation->SetupAttachment(RootComponent);

	//SpawnLocation += GetActorLocation();
}

void ABoxSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnAI = GetWorld()->GetSubsystem<USpawnAI>();
	
	if (SpawnOnPoint)
		LoopSpawnBox(SpawnRate);
}

AItem* ABoxSpawner::SpawnItem()
{
	if (IsOldBoxSpawner)
	{
		FTransform SpawnTransform = FTransform(FRotator::ZeroRotator, SpawnLocation->GetComponentLocation());
	
		TSubclassOf<AActor> ItemToSpawn = BoxToSpawn;
		bool IsSuspicious = ShouldHappen(SuspiciousBoxSpawnRate);
		bool IsDangerous = ShouldHappen(DangerousBoxSpawnRate);
	
		if (IsSuspicious)
		{		
			if (IsDangerous)
			{
				ItemToSpawn = BombToSpawn;
			}
		}
	
		// Sets all properties of an item before spawning it
		AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(ItemToSpawn, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		AItem* Item = Cast<AItem>(NewActor);
	
		if (Item)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Spawning Box"));
			Item->SetIsLarge(ShouldHappen(LargeBoxSpawnRate));
			Item->SetIsFragile(ShouldHappen(FragileBoxSpawnRate));
			//Item->SetIsDangerous(ShouldHappen(DangerousBoxSpawnRate));
			//Item->SetActorRotation(SpawnLocation->GetComponentRotation() + FRotator(0, FMath::RandRange(-15,15), 0));
			Item->SetIsSuspicious(IsSuspicious);
			Item->SetIsDangerous(IsDangerous);
			Item->SetPlaySound(PlayBoxSound);
		}

		// Actually spawn item
		UGameplayStatics::FinishSpawningActor(Item, SpawnTransform);
		return Item;
	}
	
	FTransform SpawnTransform = FTransform(FRotator::ZeroRotator, SpawnLocation->GetComponentLocation());
	
	TSubclassOf<AActor> ItemToSpawn = BoxToSpawn;
	TArray<EBoxType> Properties = SpawnAI->ConstructBox();
	
	if (Properties.IsEmpty())
	{
		return nullptr;
	}
	
	if (Properties.Contains(EBoxType::Dangerous))
	{
		ItemToSpawn = BombToSpawn;
	}

	// Sets all properties of an item before spawning it
	AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(ItemToSpawn, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	AItem* Item = Cast<AItem>(NewActor);
	
	if (Item)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Spawning Box"));
		Item->SetIsLarge(Properties.Contains(EBoxType::Large));
		Item->SetIsFragile(Properties.Contains(EBoxType::Fragile));
		//Item->SetIsDangerous(ShouldHappen(DangerousBoxSpawnRate));
		//Item->SetActorRotation(SpawnLocation->GetComponentRotation() + FRotator(0, FMath::RandRange(-15,15), 0));
		Item->SetIsSuspicious(Properties.Contains(EBoxType::Suspicious));
		Item->SetIsDangerous(Properties.Contains(EBoxType::Dangerous));
		
		Item->SetAddress(SpawnAI->SetAddress());
		
		Item->SetPlaySound(PlayBoxSound);
	}

	// Actually spawn item
	UGameplayStatics::FinishSpawningActor(Item, SpawnTransform);
	return Item;
}

AItem* ABoxSpawner::SpawnBox()
{
	return SpawnItem();
}

AItem* ABoxSpawner::SpawnItem(bool IsDangerous, bool IsLarge, bool IsFragile, bool IsSuspicious, EBoxAddress Address)
{
	FTransform SpawnTransform = FTransform(FRotator::ZeroRotator, SpawnLocation->GetComponentLocation());
	
	TSubclassOf<AActor> ItemToSpawn = BoxToSpawn;
	
	if (IsSuspicious)
	{		
		if (IsDangerous)
		{
			ItemToSpawn = BombToSpawn;
		}
	}
	
	// Sets all properties of an item before spawning it
	AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(ItemToSpawn, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	AItem* Item = Cast<AItem>(NewActor);
	
	if (Item)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Spawning Box"));
		Item->SetIsLarge(IsLarge);
		Item->SetIsFragile(IsFragile);
		//Item->SetIsDangerous(ShouldHappen(DangerousBoxSpawnRate));
		//Item->SetActorRotation(SpawnLocation->GetComponentRotation() + FRotator(0, FMath::RandRange(-15,15), 0));
		Item->SetIsSuspicious(IsSuspicious);
		Item->SetIsDangerous(IsDangerous);
		Item->SetAddress(Address);
		Item->SetPlaySound(PlayBoxSound);
	}

	// Actually spawn item
	UGameplayStatics::FinishSpawningActor(Item, SpawnTransform);
	return Item;
}

void ABoxSpawner::SpawnBoxOnPoint()
{
	SpawnItem();
}

void ABoxSpawner::LoopSpawnBox(float NewSpawnRate)
{
	GetWorld()->GetTimerManager().SetTimer(
		SpawnRateTimer,
		this,
		&ABoxSpawner::SpawnBoxOnPoint,
		NewSpawnRate,
		true
		);
}

bool ABoxSpawner::ShouldHappen(int Percentage)
{
	if (Percentage <= 0)
	{
		return false;
	}
	return (FMath::RandRange(1, 100/Percentage) == 1 ? true : false);
}

// I hate this solution
EBoxAddress ABoxSpawner::SetBoxAddress()
{
	int number = FMath::RandRange(0, 2);
	
	switch (number)
	{
		case 0:
			return EBoxAddress::CIRCLE;
		case 1:
			return EBoxAddress::SQUARE;
		// case 2:
		// 	return BoxAddress::TRIANGLE;
		default:
			return EBoxAddress::CIRCLE;
	}
	
	//uint8 hks = FMath::RandHelper(BoxAddress::TOTAL_COUNT);
	// return (FMath::RandRange(1, 100/Percentage) == 1 ? true : false)
}
