// Marcus hopefully approves of this.

#include "ConveyorBeltUpgraded.h"

// Sets default values
AConveyorBeltUpgraded::AConveyorBeltUpgraded()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//subobject needs to be created here
	Path =  CreateDefaultSubobject<USplineComponent>("Path",true);
}

// Called when the game starts or when spawned
void AConveyorBeltUpgraded::BeginPlay()
{
	Super::BeginPlay();


	for (int i = 0; i < Segments.Num(); i++)
	{
		if (!Segments[i])
		{
			bIsOn = false;
			UE_LOG(LogTemp, Error, TEXT("Can't create Belt. YOU ARE MISSING A SEGMENT IN THE CONVEYOR BELT: %s at index: %d"), *this->GetActorNameOrLabel(), i);
			return;
		}
		
		//Segments[i]->Belt = this;
		Segments[i]->BeltUpgraded = this;
		Segments[i]->IndexInConveyorBelt=i;
	}

	
	GenerateSpline();
}

// Called every frame
void AConveyorBeltUpgraded::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsOn)
	{
		Move();
	}
}

void AConveyorBeltUpgraded::RemoveFromBelt(AItem* Item)
{
	//UE_LOG(LogTemp, Log, TEXT("RemoveFromBelt with Item"));
	FObjectOnBelt* Current = First;
	FObjectOnBelt* Previous = nullptr;
	while (Current)
	{
		if (Current->Item == Item)
		{
			if (Previous)
				Previous->Next = Current->Next;
			if (Current == First)
			{
				First = Current->Next;
			}
			Item->SetPhysics(true);
			Item->SetCollitionDefultProfile(true);
			Item ->Conveyor = nullptr; //sluta peka på denna conveyor
			Item->ConveyorUpgraded = nullptr;
			//Current -> Destroy();
			//Destroy(Current);
			break;
		}
		Previous = Current;
		Current = Current->Next;
	}
}
void AConveyorBeltUpgraded::RemoveFromBelt(FObjectOnBelt* Obj)
{
	//UE_LOG(LogTemp, Log, TEXT("RemoveFromBelt with struct"));
	FObjectOnBelt* Current = First;
	FObjectOnBelt* Previous = nullptr;
	while (Current)
	{
		if (Current == Obj)
		{
			if (Previous)
				Previous->Next = Current->Next;
			if (Current == First)
			{
				First = Current->Next;
			}
			Current->Item->SetPhysics(true);
			Current->Item->SetCollitionDefultProfile(true);
			Current->Item ->Conveyor = nullptr; //sluta peka på denna conveyor
			Current->Item->ConveyorUpgraded = nullptr;
			//Current -> Destroy();
			//Destroy(Current);
			break;
		}
		Previous = Current;
		Current = Current->Next;
	}
}

void AConveyorBeltUpgraded::AddToBelt(AItem* Item)
{
	//UE_LOG(LogTemp, Log, TEXT("Add to belt"));
	//Create ObjectOnBelt
	float MovedDelta = Path->FindInputKeyClosestToWorldLocation(Item->GetActorLocation());
	FVector Offset = FVector(0,0,0);
	if (abs(MovedDelta) > 0.2)
		Offset += Item->GetActorLocation() - Path->GetLocationAtSplineInputKey(MovedDelta,ESplineCoordinateSpace::World);
	if (abs(Offset.X) > 100 || abs(Offset.Y) > 100 || abs(Offset.Z)>50) //lägg inte till om för långt ute på kanten
		return;
	Item->SetActorRotation(FRotator(0, Item->GetActorRotation().Yaw, 0));
	
	FObjectOnBelt* Obj = new FObjectOnBelt(Item,nullptr, MovedDelta,Offset);
	//Item->Conveyor = this; 
	Item->ConveyorUpgraded = this;
	Item->SetPhysics(false);	
	Item->SetCollitionDefultProfile(false);
	
	if (First==nullptr || Obj->MovedDelta < 1) //moved delta grejen kan tas bort, bara här temporarily
	{
		Obj->Next = First;
		First = Obj;
		return;
	}
	
	FObjectOnBelt* Current = First;
	FObjectOnBelt* Previous = nullptr;
	while (Current)
	{
		if (Current->MovedDelta > Obj->MovedDelta || !Current->Next)
		{
			if (Previous)
				Previous->Next = Obj;
			Obj->Next = Current;
			if (Current == First)
			{
				First = Obj;
			}
			break;
		}
		Previous = Current;
		Current = Current->Next;
	}
	
}

bool AConveyorBeltUpgraded::ItemIsAtEndOfBelt(AItem* Item)
{
	if (Path->FindInputKeyClosestToWorldLocation(Item->GetActorLocation()) >= Path->GetNumberOfSplinePoints()-1)
		return true;
	return false;
}

void AConveyorBeltUpgraded::Move()
{
	if (!First && SpawnItemsSelf)
	{
		UE_LOG(LogTemp, Warning, TEXT("First item doesnt exist!"));
		AItem* NewItem = BoxSpawner->SpawnBox();
		AddToBelt(NewItem);
	}
	
	FObjectOnBelt* Current = First;
	while (Current) //OM DET INTE FINNS NÅGOT ITEM KOMMER DEN INTE ATT RÖRA PÅ SIG!!!
	{
		Current->MovedDelta += Speed * GetWorld()->GetDeltaSeconds();
		
		//åk dubbelt så långt vid slutet då pathen är hälften så lång
		if (Current->MovedDelta >= Path->GetNumberOfSplinePoints()-2)
			Current->MovedDelta += Speed * GetWorld()->GetDeltaSeconds();
		
		float NewKey = Current->MovedDelta;
		FVector NewLoc = Path->GetLocationAtSplineInputKey(NewKey,ESplineCoordinateSpace::World);
		AItem* Item = Current->Item;
		if (IsValid(Item))
			Item->SetActorLocation(NewLoc + Current->Offset);
		
		//Rotera objekten?
		//if (Item)
		//Item->SetActorRotation(Path->GetRotationAtSplineInputKey(SegmentIndex+MovedDelta,ESplineCoordinateSpace::World));
		
		if (First!= nullptr && First->MovedDelta >= 1 && SpawnItemsSelf)
		{
			AItem* NewItem = BoxSpawner->SpawnBox();
			AddToBelt(NewItem);
		}
		
		if (Current->MovedDelta >= Path->GetNumberOfSplinePoints()-1)
		{
			RemoveFromBelt(Current);
		}
		
		Current = Current->Next;
	}
	

}

//Går igenom listan för att hitta det objekt som håller Item
FObjectOnBelt* AConveyorBeltUpgraded::FindObjectOnBelt(AItem* Item)
{
	FObjectOnBelt* Current = First;
	while (Current)
	{
		if (Current->Item == Item)
			return Current;
		
		Current = Current->Next;
	}
	return nullptr;
}

void AConveyorBeltUpgraded::GenerateSpline()
{
	Path -> ClearSplinePoints();
	//för varje conveyor segment, lägg till som punkt på path
	for (int i = 0; i < Segments.Num(); i++)
	{
		Path->AddSplinePointAtIndex(Segments[i]->GetActorLocation() + PathOffset,i,ESplineCoordinateSpace::World);
		Path -> SetSplinePointType(i,ESplinePointType::Linear,true);
	}
	
	//lägg till punkt utanför arrayen för offset där de ska falla
	FVector SegmentOrigin;
	FVector SegmentBoxExtent;
	Segments[0]->GetActorBounds(false, SegmentOrigin, SegmentBoxExtent);
	FVector DirOffset = FVector(0,0,0) + (Segments[Segments.Num()-1] -> GetForward() * (SegmentBoxExtent.X-GuardRailLengthOffset));
	
	Path-> AddSplinePointAtIndex(Segments[Segments.Num()-1]->GetActorLocation() + DirOffset  + PathOffset,Segments.Num(),ESplineCoordinateSpace::World);
	Path -> SetSplinePointType(Segments.Num(),ESplinePointType::Linear,true);
	
}





