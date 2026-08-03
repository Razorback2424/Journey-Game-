#include "Overworld/JGMapTransitionTriggerBase.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Core/JGLog.h"
#include "Engine/GameInstance.h"
#include "Overworld/JGOverworldSubsystem.h"

AJGMapTransitionTriggerBase::AJGMapTransitionTriggerBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Trigger->SetupAttachment(Root);
	Trigger->InitBoxExtent(FVector(32.0f, 32.0f, 32.0f));
	Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Trigger->SetCollisionObjectType(ECC_WorldDynamic);
	Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Trigger->SetGenerateOverlapEvents(true);
}

void AJGMapTransitionTriggerBase::BeginPlay()
{
	Super::BeginPlay();

	if (Trigger)
	{
		Trigger->OnComponentBeginOverlap.AddUniqueDynamic(this, &AJGMapTransitionTriggerBase::HandleTriggerBeginOverlap);
	}
}

bool AJGMapTransitionTriggerBase::CanTriggerTransition(AActor* TriggeringActor) const
{
	return bEnabled
		&& !bConsumed
		&& IsValid(TriggeringActor)
		&& TriggeringActor != this
		&& !DestinationMapId.IsNone();
}

bool AJGMapTransitionTriggerBase::TryRequestTransition(AActor* TriggeringActor)
{
	if (!CanTriggerTransition(TriggeringActor))
	{
		if (bDebugLog)
		{
			UE_LOG(LogJourneyOverworld, Verbose, TEXT("Transition trigger %s rejected trigger from %s"),
				*GetName(),
				*GetNameSafe(TriggeringActor));
		}
		return false;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UJGOverworldSubsystem* OverworldSubsystem = GameInstance->GetSubsystem<UJGOverworldSubsystem>())
		{
			if (OverworldSubsystem->RequestMapTransition(BuildTransitionRequest()))
			{
				if (bOneShot)
				{
					bConsumed = true;
				}

				if (bDebugLog)
				{
					UE_LOG(LogJourneyOverworld, Log, TEXT("Transition trigger %s accepted trigger from %s"),
						*GetName(),
						*GetNameSafe(TriggeringActor));
				}

				BP_OnTransitionRequested(TriggeringActor);
				return true;
			}
		}
	}

	if (bDebugLog)
	{
		UE_LOG(LogJourneyOverworld, Verbose, TEXT("Transition trigger %s request not accepted"), *GetName());
	}

	return false;
}

FJGMapTransitionRequest AJGMapTransitionTriggerBase::BuildTransitionRequest() const
{
	FJGMapTransitionRequest Request;
	Request.MapId = DestinationMapId;
	Request.SpawnTag = DestinationSpawnTag;
	Request.bSeamless = bSeamlessTravel;
	Request.DebugReason = DebugReason;
	return Request;
}

void AJGMapTransitionTriggerBase::HandleTriggerBeginOverlap(
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

	TryRequestTransition(OtherActor);
}
