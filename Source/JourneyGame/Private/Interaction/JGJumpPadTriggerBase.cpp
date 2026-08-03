#include "Interaction/JGJumpPadTriggerBase.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Core/JGLog.h"
#include "GameFramework/Character.h"

AJGJumpPadTriggerBase::AJGJumpPadTriggerBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Trigger->SetupAttachment(Root);
	Trigger->InitBoxExtent(FVector(32.0f, 32.0f, 16.0f));
	Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Trigger->SetCollisionObjectType(ECC_WorldDynamic);
	Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Trigger->SetGenerateOverlapEvents(true);
}

void AJGJumpPadTriggerBase::BeginPlay()
{
	Super::BeginPlay();

	if (Trigger)
	{
		Trigger->OnComponentBeginOverlap.AddUniqueDynamic(this, &AJGJumpPadTriggerBase::HandleTriggerBeginOverlap);
	}
}

bool AJGJumpPadTriggerBase::CanLaunchActor(AActor* OtherActor) const
{
	return bEnabled
		&& !bConsumed
		&& IsValid(OtherActor)
		&& OtherActor != this
		&& Cast<ACharacter>(OtherActor) != nullptr;
}

bool AJGJumpPadTriggerBase::TryLaunchActor(AActor* OtherActor)
{
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!CanLaunchActor(Character))
	{
		return false;
	}

	Character->LaunchCharacter(LaunchVelocity, bOverrideXY, bOverrideZ);

	if (bOneShot)
	{
		bConsumed = true;
	}

	if (bDebugLog)
	{
		UE_LOG(LogJourneyInteraction, Log, TEXT("Jump pad %s launched %s"), *GetName(), *GetNameSafe(Character));
	}

	BP_OnActorLaunched(Character);
	return true;
}

void AJGJumpPadTriggerBase::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	(void)OverlappedComponent;
	(void)OtherComp;
	(void)OtherBodyIndex;
	(void)bFromSweep;
	(void)SweepResult;

	TryLaunchActor(OtherActor);
}
