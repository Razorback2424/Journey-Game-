#include "Encounters/JGEncounterZoneBase.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Core/JGLog.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Encounters/JGEncounterTableDataAsset.h"
#include "Overworld/JGOverworldSubsystem.h"

AJGEncounterZoneBase::AJGEncounterZoneBase()
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

void AJGEncounterZoneBase::BeginPlay()
{
	Super::BeginPlay();

	if (Trigger)
	{
		Trigger->OnComponentBeginOverlap.AddUniqueDynamic(this, &AJGEncounterZoneBase::HandleTriggerBeginOverlap);
	}
}

UJGEncounterTableDataAsset* AJGEncounterZoneBase::GetEncounterTable_Implementation() const
{
	return EncounterTable;
}

bool AJGEncounterZoneBase::CanTriggerEncounter_Implementation(AActor* TriggeringActor) const
{
	if (!bEnabled || bConsumed)
	{
		return false;
	}
	if (!IsValid(TriggeringActor) || TriggeringActor == this)
	{
		return false;
	}
	if (!EncounterTable || !EncounterTable->bEnabled)
	{
		return false;
	}
	return true;
}

void AJGEncounterZoneBase::HandleEncounterConsumed_Implementation(AActor* TriggeringActor)
{
	if (bOneShot)
	{
		bConsumed = true;
	}

	if (bDebugLog)
	{
		UE_LOG(LogJourneyEncounter, Log, TEXT("Encounter zone %s consumed by %s"), *GetName(), *GetNameSafe(TriggeringActor));
	}
}

void AJGEncounterZoneBase::HandleTriggerBeginOverlap(
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

	if (!IJGEncounterSourceInterface::Execute_CanTriggerEncounter(this, OtherActor))
	{
		return;
	}

	if (bDebugLog)
	{
		UE_LOG(LogJourneyEncounter, Log, TEXT("Encounter zone %s overlap by %s"), *GetName(), *GetNameSafe(OtherActor));
	}

	bool bAccepted = false;
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UJGOverworldSubsystem* OverworldSubsystem = GameInstance->GetSubsystem<UJGOverworldSubsystem>())
		{
			FJGEncounterRequest Request;
			Request.SourceActor = this;
			Request.TriggeringActor = OtherActor;
			Request.EncounterTable = EncounterTable;
			Request.RequestedAtSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
			bAccepted = OverworldSubsystem->RequestEncounter(Request);
		}
	}

	if (bAccepted)
	{
		IJGEncounterSourceInterface::Execute_HandleEncounterConsumed(this, OtherActor);
		BP_OnEncounterTriggered(OtherActor);
	}
	else if (bDebugLog)
	{
		UE_LOG(LogJourneyEncounter, Verbose, TEXT("Encounter zone %s request not accepted"), *GetName());
	}
}
