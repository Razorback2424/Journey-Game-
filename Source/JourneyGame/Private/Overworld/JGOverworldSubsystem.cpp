#include "Overworld/JGOverworldSubsystem.h"

#include "Core/JGLog.h"
#include "Encounters/JGEncounterTableDataAsset.h"

void UJGOverworldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogJourneyOverworld, Log, TEXT("Overworld subsystem initialized"));
}

void UJGOverworldSubsystem::Deinitialize()
{
	UE_LOG(LogJourneyOverworld, Log, TEXT("Overworld subsystem deinitialized"));
	Super::Deinitialize();
}

void UJGOverworldSubsystem::SetCurrentMapId(const FName InMapId)
{
	CurrentMapId = InMapId;
}

FName UJGOverworldSubsystem::GetCurrentMapId() const
{
	return CurrentMapId;
}

void UJGOverworldSubsystem::SetNextSpawnTag(const FName InSpawnTag)
{
	NextSpawnTag = InSpawnTag;
}

FName UJGOverworldSubsystem::GetNextSpawnTag() const
{
	return NextSpawnTag;
}

void UJGOverworldSubsystem::SetEncountersEnabled(const bool bInEnabled)
{
	bEncountersEnabled = bInEnabled;
}

bool UJGOverworldSubsystem::AreEncountersEnabled() const
{
	return bEncountersEnabled;
}

bool UJGOverworldSubsystem::RequestEncounter(const FJGEncounterRequest& Request)
{
	if (!bEncountersEnabled)
	{
		UE_LOG(LogJourneyEncounter, Verbose, TEXT("Encounter request rejected: encounters disabled"));
		return false;
	}

	if (!Request.HasMinimumData())
	{
		UE_LOG(LogJourneyEncounter, Warning, TEXT("Encounter request rejected: missing encounter table"));
		return false;
	}

	if (bHasPendingEncounter)
	{
		UE_LOG(LogJourneyEncounter, Verbose, TEXT("Encounter request rejected: pending request already exists"));
		return false;
	}

	PendingEncounterRequest = Request;
	bHasPendingEncounter = true;

	UE_LOG(LogJourneyEncounter, Log, TEXT("Encounter requested (source=%s, trigger=%s, table=%s)"),
		*GetNameSafe(Request.SourceActor),
		*GetNameSafe(Request.TriggeringActor),
		*GetNameSafe(Request.EncounterTable.Get()));

	return true;
}

bool UJGOverworldSubsystem::ConsumePendingEncounter(FJGEncounterRequest& OutRequest)
{
	if (!bHasPendingEncounter)
	{
		return false;
	}

	OutRequest = PendingEncounterRequest;
	ClearPendingEncounter();
	return true;
}

void UJGOverworldSubsystem::ClearPendingEncounter()
{
	bHasPendingEncounter = false;
	PendingEncounterRequest = FJGEncounterRequest{};
}

bool UJGOverworldSubsystem::HasPendingEncounter() const
{
	return bHasPendingEncounter;
}

bool UJGOverworldSubsystem::RequestMapTransition(const FJGMapTransitionRequest& Request)
{
	if (!Request.IsValidRequest())
	{
		UE_LOG(LogJourneyOverworld, Warning, TEXT("Map transition request rejected: invalid request"));
		return false;
	}

	if (bHasPendingMapTransition)
	{
		UE_LOG(LogJourneyOverworld, Verbose, TEXT("Map transition request rejected: pending request already exists"));
		return false;
	}

	PendingMapTransitionRequest = Request;
	PendingMapTransitionRequest.Status = EJGTransitionRequestStatus::Pending;
	bHasPendingMapTransition = true;

	UE_LOG(LogJourneyOverworld, Log, TEXT("Map transition requested (MapId=%s, SpawnTag=%s, Seamless=%s)"),
		*PendingMapTransitionRequest.MapId.ToString(),
		*PendingMapTransitionRequest.SpawnTag.ToString(),
		PendingMapTransitionRequest.bSeamless ? TEXT("true") : TEXT("false"));

	if (!PendingMapTransitionRequest.SpawnTag.IsNone())
	{
		NextSpawnTag = PendingMapTransitionRequest.SpawnTag;
	}

	return true;
}

bool UJGOverworldSubsystem::ConsumePendingMapTransition(FJGMapTransitionRequest& OutRequest)
{
	if (!bHasPendingMapTransition)
	{
		return false;
	}

	OutRequest = PendingMapTransitionRequest;
	OutRequest.Status = EJGTransitionRequestStatus::Consumed;
	ClearPendingMapTransition();
	return true;
}

void UJGOverworldSubsystem::ClearPendingMapTransition()
{
	bHasPendingMapTransition = false;
	PendingMapTransitionRequest = FJGMapTransitionRequest{};
}

bool UJGOverworldSubsystem::HasPendingMapTransition() const
{
	return bHasPendingMapTransition;
}
