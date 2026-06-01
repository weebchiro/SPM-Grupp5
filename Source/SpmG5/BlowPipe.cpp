// Marcus hopefully approves of this.

#include "BlowPipe.h"

#include <ThirdParty/ShaderConductor/ShaderConductor/External/DirectXShaderCompiler/include/dxc/DXIL/DxilConstants.h>

#include "SpmG5Character.h"
#include "DSP/BufferDiagnostics.h"

// Sets default values
ABlowPipe::ABlowPipe()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	//SetRootComponent(SceneComponent);
	BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
	SceneComponent->SetupAttachment(BoxMesh);
	
}

// Called when the game starts or when spawned
void ABlowPipe::BeginPlay()
{
	Super::BeginPlay();
	//Instantiete varibles
	TimeBeforeBlowing = WaitTimeRange.Y;
	TimeLeftBlowing = Stream.FRandRange(WaitTimeRange.X, WaitTimeRange.Y);
	//Starting Loop
	BlowStateLoop(TimeBeforeBlowing);
}

void ABlowPipe::Blow(float DeltaTime)
{
	//Doing sweep
	// TArray<FHitResult> HitResult;
	// FVector Location = GetActorLocation() + Offcset * GetActorForwardVector();	
	// FVector End = Location * GetActorForwardVector() * 5;
	// FCollisionShape Box = FCollisionShape::MakeBox(BlowBoxSize);
	// FQuat Rotation = GetActorRotation().Quaternion();
	// GetWorld()->SweepMultiByChannel(HitResult,Location, End, Rotation, ECC_GameTraceChannel4,Box);
	//
	// TimeLeftBlowing -= DeltaTime;
	//
	// DrawDebugBox(GetWorld(), Location, FVector(BlowBoxSize.Y, BlowBoxSize.X, BlowBoxSize.Z), FColor::Red, false, 1);
	// //Moves components
	// for (FHitResult Result : HitResult)
	// {
	// 	Result.GetComponent()->GetOwner()->SetActorLocation(Result.GetComponent()->GetOwner()->GetActorLocation() + GetActorForwardVector() * ForceMultiplier * DeltaTime);
	// }
	//
}

void ABlowPipe::StartBlowing()
{
	UE_LOG(LogTemp, Warning, TEXT("Blowing!!!"));
	GetWorldTimerManager().ClearTimer(IdleTimer);	
	//Starts blowing every second
	GetWorld()->GetTimerManager().SetTimer(BlowingTimer, this, &ABlowPipe::CallBlowMethod, GetWorld()->GetDeltaSeconds(), true, 1);
}

void ABlowPipe::CallBlowMethod()
{	
	//Stop Showing Indicator
	UE_LOG(LogTemp, Warning, TEXT("Blowing Loop! Time left: %f"), TimeLeftBlowing);
	if (TimeLeftBlowing > 0)
	{
		ActivateBlowing(true);
		TimeLeftBlowing -= GetWorld()->GetDeltaSeconds();
	}
	else
	{		
		//Resett Timer and stop blowing
		ActivateBlowing(false);
		GetWorldTimerManager().ClearTimer(BlowingTimer);
		UE_LOG(LogTemp, Warning, TEXT("Ended blowing:("));
		BlowStateLoop(TimeLeftBlowing = Stream.FRandRange(WaitTimeRange.X, WaitTimeRange.Y));	
	}
}

void ABlowPipe::BlowStateLoop(float LoopRate)
{
	UE_LOG(LogTemp, Warning, TEXT("New Blow Loop!! Time to wait: %f"), TimeLeftBlowing);
	GetWorld()->GetTimerManager().SetTimer(IdleTimer, this, &ABlowPipe::StartBlowing, LoopRate, true, LoopRate);
	GetWorld()->GetTimerManager().SetTimer(IndicatorTimer, this, &ABlowPipe::ShowIndicator, LoopRate - BlowIndicatorTime, true, LoopRate- BlowIndicatorTime);
}

void ABlowPipe::ShowIndicator()
{
	GetWorldTimerManager().ClearTimer(IndicatorTimer);	
	ShowIndication();
	UE_LOG(LogTemp, Warning, TEXT("Showing Indicator"));
}

// Called every frame
void ABlowPipe::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// if (Blowing)
	// 	Blow(DeltaTime);
}

