// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "ConveyorBelt.h"
#include "FMODBlueprintStatics.h"
#include "Interactable.h"
#include "NiagaraFunctionLibrary.h"
#include "SpmG5Character.h"
#include "StateTreeTypes.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "GameFramework/PawnMovementComponent.h"

// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	//BaseStaticMesh = CreateDefaultSubobject<USceneComponent>("BaseStaticMesh");
	
	SetRootComponent(BaseMesh);
	//BaseMesh->SetupAttachment(BaseStaticMesh);

	PrimComp = Cast<UPrimitiveComponent>(BaseMesh);
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	
	BaseMesh->OnComponentHit.AddDynamic(this, &AItem::OnHit);
	
	SetPoints();
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (CollisionSFXTimer > 0)
	{
		CollisionSFXTimer -= DeltaTime;
	}
}

void AItem::SetPoints()
{
	Points = 0;
	
	if (IsDangerous)
	{
		SetNegativePoints();
		return;
	}
	
	if (IsLarge)
	{
		Points += LargeBoxPoints;
	}
	else
	{
		Points += SmallBoxPoints;
	}
	
	if (IsFragile)
	{
		Points += FragileBoxPoints;
	}
	
	if (IsScanned)
	{
		Points += ScannedBoxPoints;
	}
	
	if (IsInspected)
	{
		Points += InspectedBoxPoints;
	}
}

void AItem::SetNegativePoints()
{
	Points = WrongBoxPoints;
}

void AItem::SetPhysics(bool SetTo)
{
	// SetActorEnableCollision(SetTo);
	PrimComp->SetEnableGravity(SetTo);
	PrimComp->SetSimulatePhysics(SetTo);
}

void AItem::SetCollitionDefultProfile(bool SetTo)
{
	if (SetTo)
		PrimComp->SetCollisionProfileName("DefaultItem");
	else 
		PrimComp->SetCollisionProfileName("OnConveyor");//OnlyRaycast
}

void AItem::CalculateIfBreakIfFragile()
{
	if (IsFragile)
	{
		// Checks with how much force fragile item hits something
		int CurrentSpeed = GetVelocity().Size();
		UE_LOG(LogTemp, Warning, TEXT("Current Speed: %d"), CurrentSpeed);
		
		if (ShouldBreakOnImpact)
		{
			Disintegrate(false);
		}
		
		if (CurrentSpeed > MaxSpeedIfFragile)
		{
			Disintegrate(false);
		}
	}
}

void AItem::Disintegrate(bool bThrownInTrash)
{
	if (bThrownInTrash)
	{
		// Box is thrown in incinerator
		if (TrashBreakParticles)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TrashBreakParticles, GetActorLocation(), GetActorRotation());
		}
	}
	else
	{

		if (IsValid(MostRecentHolder))
		{
			if (ASpmG5Character* Char = Cast<ASpmG5Character>(MostRecentHolder))
			{
				if (Char->GetItem() == this)
				{
					Char->Drop();
					UE_LOG(LogTemp, Error, TEXT("Item dropped from player"));
				}
			}
			else if (MostRecentHolder->Implements<UInteractable>())
			{
				UObject* StationHit = Cast<UObject>(MostRecentHolder);
				//StationHit->k
				//DEALA MED ATT TA BORT LYSANDE STUFF
			}
		}
		
		
		// Box breaks (it's fragile)
		if (FragileBreakParticles)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), FragileBreakParticles, GetActorLocation(), GetActorRotation());
		}
	}
	
	//Play SFX - Ruben
	if (PlaySound) //TA BORT EFTER SPELTEST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	{
		UFMODBlueprintStatics::PlayEventAtLocation(this, DestroySFX, FTransform(GetActorLocation()), true);
		
		if (CollisionSFXInstance)
		{
			CollisionSFXInstance->Stop();
			CollisionSFXInstance->Release();
			CollisionSFXInstance = nullptr;
		}
	}
	
	Destroy();
}

void AItem::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != this)
	{
		//MostRecentHolder excludes person who was holding item so it can't destroy item right after dropping it
		if (MostRecentHolder)
		{
			if (OtherActor != MostRecentHolder)
			{
				MostRecentHolder = nullptr;
				CalculateIfBreakIfFragile();
			}
		}
		
		//FIXA EFTER SPLETEST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// ^^^ Kommentaren ovan pratar om PlaySound ^^^
		if (PlaySound && CollisionSFXTimer <= 0 && !OtherActor->IsA(AItem::StaticClass())) 
		{
			//Play Collision SFX. Volume is based on how big the impact was - Ruben
			float Magnitude = NormalImpulse.Size();
		
			if (Magnitude > SFXNormalImpulseMin)
			{
				float Volume = FMath::GetMappedRangeValueClamped(
					FVector2D(SFXNormalImpulseMin, SFXNormalImpulseMax),
					FVector2D(SFXCollisionVolumeMin, SFXCollisionVolumeMax),
					Magnitude
				);
		
				if (Volume <= 0) { return; }
			
				// Ignore first play as it's from when the box is spawned in the conveyor - Ruben
				CollisionSoundsPlayed++;
				
				CollisionSFXTimer = CollisionSFXCooldown;
			
				if (CollisionSoundsPlayed > 1)
				{
					//UE_LOG(LogTemp, Warning, TEXT("Item was HIT with force of %f!!! Playing with a volume of %f"), Magnitude, Volume);
		
					if (CollisionSFXInstance)
					{
						CollisionSFXInstance->Stop();
						CollisionSFXInstance->Release();
						CollisionSFXInstance = nullptr;
					}
					
					CollisionSFXInstance = UFMODBlueprintStatics::PlayEventAttached(
						CollisionSFX, 
						BaseMesh, 
						NAME_None, 
						FVector::ZeroVector, 
						EAttachLocation::KeepRelativeOffset, 
						true, 
						true, 
						true
						);
					
					if (CollisionSFXInstance)
					{
						CollisionSFXInstance->SetVolume(Volume);
					}
				}
			}
		}
	}
}

void AItem::AddImpulse(FVector Point, float Strength)
{
	//hitta riktning mellan point och this object
	FVector Imp = GetActorLocation() - Point;
	//för att få 1 längd på vektor
	//och 2 riktning
	//lägg velocity på detta objekt
	//i riktningen * strength (/längd på vektor, så längre bort blir påverkad mindre)
	
	UE_LOG(LogTemp, Warning, TEXT("Impulse on object!"));
	//DrawDebugSphere(GetWorld(), GetActorLocation(), 50, 20, FColor::Blue, false, 0.2);
	//this->AddVelocity(10);
	//this->AddVelocity(Strength);
	FVector Impulse = FVector(1,2,1);
	//BaseMesh->AddForce(BaseMesh->GetBodyInstance()->GetBodyMass() * Impulse * 1000);
	PrimComp->AddImpulse(Imp*30 * Strength * Impulse);
}

// For audio purposes - Ruben
int AItem::GetAudioType()
{
	return static_cast<int>(AudioType);
}

void AItem::SetMostRecentHolder(AActor* Holder)
{
	MostRecentHolder = Holder;
	//return (MostRecentHolder) ? MostRecentHolder : nullptr;
}

void AItem::SetIsLarge(bool SetTo)
{
	IsLarge = SetTo;
}

void AItem::SetIsFragile(bool SetTo)
{
	IsFragile = SetTo;
}

void AItem::SetIsSuspicious(bool SetTo)
{
	IsSuspicious = SetTo;
}

void AItem::SetIsDangerous(bool SetTo)
{
	IsDangerous = SetTo;
}

void AItem::SetAddress(EBoxAddress NewAddress)
{
	Address = NewAddress;
}

void AItem::SetIsScanned(bool SetTo)
{
	IsScanned = SetTo;
}

void AItem::SetIsInspected(bool SetTo)
{
	IsInspected = SetTo;
}

void AItem::SetIsHeld(bool SetTo)
{
	IsHeld = SetTo;
}

void AItem::SetHasBeenDroppedOff(bool SetTo)
{
	HasBeenDroppedOff = SetTo;
}
