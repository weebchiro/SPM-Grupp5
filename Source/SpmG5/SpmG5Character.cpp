
#include "SpmG5Character.h"
#include "Item.h"
#include "Engine/LocalPlayer.h"
//#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
//#include "GameFramework/SpringArmComponent.h"
#include "ConveyorBelt.h"
#include "ConveyorBeltUpgraded.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#if WITH_EDITOR
#endif
#include "SpmG5.h"
#include "FMODBlueprintStatics.h"
#include "Interactable.h"

ASpmG5Character::ASpmG5Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	HoldingLocation = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HoldingLocation"));
	HoldingLocation->SetupAttachment(RootComponent);
	
	PrimComp = Cast<UPrimitiveComponent>(GetCapsuleComponent());
	
	//OM vi vill ha den i timer så får vi fixa det här
	// GetWorld()->GetTimerManager().SetTimer(OutlineUpdateTimer, this, &ASpmG5Character::DoSweep(false), OutlineUpdateRate, true, 1);
	
}

void ASpmG5Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASpmG5Character::Move);

		// Pickup and Drop
		EnhancedInputComponent->BindAction(PickupOrDropAction, ETriggerEvent::Started, this, &ASpmG5Character::ChooseInteractOrPickup);//PickupAndDrop
		EnhancedInputComponent->BindAction(ThrowAction, ETriggerEvent::Triggered, this, &ASpmG5Character::ChargeUpThrow);		
		EnhancedInputComponent->BindAction(ThrowAction, ETriggerEvent::Completed, this, &ASpmG5Character::Throw);
		
		// Pause menu navigation
		EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &ASpmG5Character::Pause);
		//interact
		//EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ASpmG5Character::Interact);
	}
	else
	{
		UE_LOG(LogSpmG5, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

TArray<FHitResult> ASpmG5Character::DoSweep()
{
    TArray<FHitResult> HitResults;
    TArray<FHitResult> HitResultsItems;
    TArray<FHitResult> HitResultsInteractable;

    float Distance = 5.0f;
    FVector Location = HoldingLocation->GetComponentLocation();
    FVector End = Location + GetActorForwardVector() * Distance;
    FCollisionShape Box = FCollisionShape::MakeBox(PickUpBoxSize);
    FQuat Rotation = GetActorRotation().Quaternion();

    GetWorld()->SweepMultiByChannel(HitResultsItems,Location, End, Rotation, ECC_GameTraceChannel1,Box);
	for (FHitResult HitResult : HitResultsItems)
		HitResults.Add(HitResult);
	GetWorld()->SweepMultiByChannel(HitResultsInteractable,Location, End, Rotation, ECC_GameTraceChannel3,Box);
	for (FHitResult HitResult : HitResultsInteractable)
	HitResults.Add(HitResult);
	
    return HitResults;
}

void ASpmG5Character::FindBoxToPickup()
{
	TArray<FHitResult> HitResults = DoSweep();
	int FirstBoxIndex = 0;
	
	for (FHitResult HitResult : HitResults)
	{
		if (Cast<AItem>(HitResult.GetActor()))		
			break;
		FirstBoxIndex++;
	}
		
	if (HitResults.IsValidIndex(FirstBoxIndex) && Cast<AItem>(HitResults[FirstBoxIndex].GetActor()))
	{
		AItem* TestCastItemToPickup = Cast<AItem>(HitResults[FirstBoxIndex].GetActor());
		if (TestCastItemToPickup != ItemToPickup)
		{
			if (ItemToPickup)
				ItemToPickup->CallActivateOvelay(false);
				
			ItemToPickup = TestCastItemToPickup;
			ItemToPickup->CallActivateOvelay(true);
		}
	}
	else if(ItemToPickup)		
		ItemToPickup->CallActivateOvelay(false);
}

void ASpmG5Character::ChooseInteractOrPickup()
{
    //Choosing between interact or pickup...
    bool bHitItem = false;
    AItem* ItemHit = nullptr;
    bool bHitStation = false;
    UObject* StationHit = nullptr;

    //Kolla igenom items och interactables in range
    for (FHitResult HitResult : DoSweep()) //Borde flyttas till hjälpmetod
    {
        if (HitResult.GetActor() && HitResult.GetComponent())
        {
            if (!ItemHit)
            {
                if (AItem* A = Cast<AItem>(HitResult.GetActor()))
                {
                    ItemHit = A;
                    bHitItem = true;
                    continue;
                }
            }

            if (HitResult.GetActor()->Implements<UInteractable>())
            {
                StationHit = Cast<UObject>(HitResult.GetActor());
                bHitStation = true;
            }
        }
    }

    //bestämm vad som ska göras
    if (bHitItem && bHitStation && !ItemHit->bIsInStation)
    {
        if (!HeldItem)
        {
            Pickup(ItemHit);
        }
        else
        {
            IsInteracting = true;
            Interact(StationHit);
        }
    }
    else if (bHitStation)
    {
        Interact(StationHit);
    }
    else if (bHitItem)
    {
        if (!HeldItem)
        {
            Pickup(ItemHit);
        }
        else
        {
            Drop();
        }
    }
}

void ASpmG5Character::Pickup(AItem* Item)
{
	Drop();
	if (!Item || Item->GetIsHeld())
		return;
	HeldItem = Item;
	HasItem = true;
	if (HeldItem->Conveyor)
	{
		HeldItem->Conveyor->DropItem(HeldItem);
		UE_LOG(LogTemp, Display, TEXT("Dropping item from conveyor"));
	}
	if (HeldItem->ConveyorUpgraded)
	{
		HeldItem->ConveyorUpgraded->RemoveFromBelt(HeldItem);
		UE_LOG(LogTemp, Display, TEXT("Dropping item from conveyor"));
	}
	if (Item->GetIsFragile())
	{
		Item->ShouldBreakOnImpact = false;
	}
	Item->SetIsHeld(true);
	
	AttachPackage();
	UE_LOG(LogTemp, Display, TEXT("%i"), Item->GetIsHeld());
}

void ASpmG5Character::AttachPackage()
{
	HeldItem->SetCollitionDefultProfile(false);
	HeldItem->SetPhysics(true);
	HeldItem->ResetVelocity();
	HeldItem->SetActorRotation(GetActorRotation()); 
	// HeldItem->SetActorRelativeRotation(FRotator(0,0,0));
	HeldItem->SetActorRelativeLocation(HoldingLocation->GetComponentLocation());
	// HeldItem->SetActorRelativeRotation(FRotator(0,0,0));
	HeldItem->SetMostRecentHolder(this);
	
	AttatchPackageToConstaint(HeldItem);
	
	// HeldItem->SetActorRelativeRotation(FRotator(0,0,0));
	// GetWorldTimerManager().SetTimer(HoldingTimer, this, &ASpmG5Character::Hold, 0.01f, true);
	
	// Play Pickup SFX
	FFMODEventInstance evt = UFMODBlueprintStatics::PlayEventAtLocation(this, PickupSFX, FTransform(GetActorLocation()), true);
	UFMODBlueprintStatics::EventInstanceSetParameter(evt, "ItemType", HeldItem->GetAudioType());
}

void ASpmG5Character::Hold()
{	
	FVector HoldingLocationWorld = HoldingLocation->GetComponentLocation();
	
	if (!IsValid(HoldingLocation) || !IsValid(HeldItem))
	{
		HeldItem = nullptr;
		return;
	}
	
	if (HeldItem)
	{
		HeldItem->SetActorLocationAndRotation(HoldingLocationWorld, GetActorRotation());	
	}
}


AItem* ASpmG5Character::Drop()
{
	if (!HeldItem)
	{
		HasItem = false; //Se till att HasItem är false när HeldItem är nullptr
		return nullptr;
	}
		
		
	//Resetting Box
	//GetWorldTimerManager().ClearTimer(HoldingTimer);
	HeldItem->SetPhysics(true);
	HeldItem->SetCollitionDefultProfile(true);
	HeldItem->ResetVelocity();	
	
	AItem* Item = HeldItem;
	
	// Play Drop SFX
	FFMODEventInstance evt = UFMODBlueprintStatics::PlayEventAtLocation(this, DropSFX, FTransform(GetActorLocation()), true);
	UFMODBlueprintStatics::EventInstanceSetParameter(evt, "ItemType", HeldItem->GetAudioType());
	
	//Reset Player
	ShowOrHideThrowBar(false);
	CurrentThrowForce = StartingThrowForce;
	HeldItem = nullptr;
	HasItem = false;
	Throwing = false;	
	DeAattatchPackageToConstaint(HeldItem);
	Item->SetIsHeld(false);
	return Item;
}

void ASpmG5Character::Pause()
{
	UGameManager* GameManager = Cast<UGameManager>(GetWorld()->GetGameInstance());

	GameManager->PauseGame();
	/*if (UGameplayStatics::IsGamePaused(this))
	{
		//UGameManager::DeactivatePauseWidget();
		UGameplayStatics::SetGamePaused(this, false);
	}
	else
	{
		//UGameManager::ActivatePauseWidget();
		UGameplayStatics::SetGamePaused(this ,true);
	}*/
}

void ASpmG5Character::Throw(const FInputActionValue& Value)
{
	if (!HeldItem)
		return;
	
	Thrown = true;
	
	ShowOrHideThrowBar(false);
	HeldItem->SetPhysics(true);
	HeldItem->SetCollitionDefultProfile(true);
	DeAattatchPackageToConstaint(HeldItem);
	HeldItem->AddVelocity(CurrentThrowForce, GetActorForwardVector());
	
	float HalfThrowForce = StartingThrowForce + ((MaxThrowForce - StartingThrowForce) * 0.66);
	
	if (HeldItem->GetIsFragile())
	{
		if (CurrentThrowForce >= HalfThrowForce)
		{
			HeldItem->ShouldBreakOnImpact = true;
		}
	}
	
	// Play Throw SFX
	FFMODEventInstance evt = UFMODBlueprintStatics::PlayEventAtLocation(this, ThrowSFX, FTransform(GetActorLocation()), true);
	UFMODBlueprintStatics::EventInstanceSetParameter(evt, "ItemType", HeldItem->GetAudioType());

	CurrentThrowForce = StartingThrowForce;
	
	//Resettar inför pickup
	HeldItem->SetIsHeld(false);
	HeldItem = nullptr;
	HasItem = false;
	Throwing = false;
	// Thrown = false;
	// GetWorldTimerManager().ClearTimer(HoldingTimer);
}

void ASpmG5Character::ChargeUpThrow(const FInputActionValue& Value)
{	
	if (!HeldItem)	
		return;			
	if (!Throwing)
		Throwing = true;
	
	ShowOrHideThrowBar(true);
	
	if (CurrentThrowForce < MaxThrowForce)
		CurrentThrowForce += ThrowForceIncrease * GetWorld()->DeltaTimeSeconds;
	UpdateThrowBar();
}

FRotator ASpmG5Character::Rotate(FVector2d Input)
{	
	float AngleRadians = FMath::Atan2(Input.X, Input.Y);
	float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);
		
	return FRotator(0.0f, AngleDegrees, 0.0f);
}

void ASpmG5Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!HeldItem)
		FindBoxToPickup();
	
	if (bIncreaseIncapacitation)
	{
		IncreaseIncapacitation();
	}
	
	DecreaseIncapacitation();
}

void ASpmG5Character::Move(const FInputActionValue& Value)
{	
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	
	//make sure controllers have some wiggleroom for stick drift
	if (MovementVector.X < StickDeadZone && MovementVector.X > -StickDeadZone)
	{
		MovementVector.X = 0;
	}
	if (MovementVector.Y < StickDeadZone && MovementVector.Y > -StickDeadZone)
	{
		MovementVector.Y = 0;
	}
	
	
	if (Throwing && (MovementVector.X != 0 || MovementVector.Y != 0)) {		
		FRotator R = FMath::Lerp(GetActorRotation(), Rotate(MovementVector), TurningSpeed/100);
		SetActorRotation(FQuat(R));
		MovementVector.X /= MovementDebufMult;
		MovementVector.Y /= MovementDebufMult;
	}
	DoMove(MovementVector.X, MovementVector.Y);	
		
}

void ASpmG5Character::IncreaseIncapacitation(float Increase)
{
	IncapacitationMeter += Increase * GetWorld()->GetDeltaSeconds();
	
	if (IncapacitationMeter > MaxIncapacitation && !bIsRagdolling)
	{
		DoRagdoll();
	}
}

void ASpmG5Character::DecreaseIncapacitation(float Decrease)
{
	if (IncapacitationMeter <= 0)
		return;
	
	if (IncapacitationMeter > 0)
		IncapacitationMeter -= Decrease * GetWorld()->GetDeltaSeconds();
	
	if (IncapacitationMeter < 0)
	{
		IncapacitationMeter = 0;
		StopRagdoll();
	}
		
}

void ASpmG5Character::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator YawRotation(0, 0, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ASpmG5Character::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void ASpmG5Character::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}
bool ASpmG5Character::GetHasItem()
{
	return HasItem;
}

AItem* ASpmG5Character::GetItem()
{
	return HeldItem;
}