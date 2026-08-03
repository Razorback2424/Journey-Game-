#include "Battle/JGBattleSubsystem.h"

#include "Battle/JGBattleSimulation.h"
#include "Battle/Data/JGBattleEncounterDefinitionDataAsset.h"
#include "Core/JGLog.h"

void UJGBattleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogJourney, Log, TEXT("Battle subsystem initialized"));
}

void UJGBattleSubsystem::Deinitialize()
{
	ClearActiveBattle();
	ClearPendingBattleStart();
	UE_LOG(LogJourney, Log, TEXT("Battle subsystem deinitialized"));
	Super::Deinitialize();
}

bool UJGBattleSubsystem::RequestBattleStart(const FJGBattleStartRequest& Request, FJGBattleCommandResult& OutResult)
{
	OutResult = FJGBattleCommandResult{};

	if (!Request.IsValidRequest())
	{
		OutResult.FailureReason = EJGBattleFailureReason::InvalidState;
		UE_LOG(LogJourney, Warning, TEXT("Battle start request rejected: missing encounter definition"));
		return false;
	}

	FString ValidationError;
	if (!Request.EncounterDefinition->ValidateDefinition(ValidationError))
	{
		OutResult.FailureReason = EJGBattleFailureReason::InvalidState;
		FJGBattleEvent& Event = OutResult.Events.AddDefaulted_GetRef();
		Event.Type = EJGBattleEventType::Message;
		Event.Message = FText::FromString(ValidationError);
		UE_LOG(LogJourney, Warning, TEXT("Battle start request rejected: %s"), *ValidationError);
		return false;
	}

	if (bHasPendingBattleStart || bHasActiveBattle)
	{
		OutResult.FailureReason = EJGBattleFailureReason::InvalidState;
		UE_LOG(LogJourney, Verbose, TEXT("Battle start request rejected: battle already pending or active"));
		return false;
	}

	PendingBattleStartRequest = Request;
	bHasPendingBattleStart = true;
	bHasCompletedBattle = false;
	LastCompletedBattleStartRequest = FJGBattleStartRequest{};

	OutResult.bSuccess = true;
	FJGBattleEvent& Event = OutResult.Events.AddDefaulted_GetRef();
	Event.Type = EJGBattleEventType::Message;
	Event.Message = FText::FromString(TEXT("Battle start requested"));

	UE_LOG(LogJourney, Log, TEXT("Battle start requested (Encounter=%s, Battle=%s, Seed=%d)"),
		*Request.SourceEncounterId.ToString(),
		*GetNameSafe(Request.EncounterDefinition),
		Request.Seed);

	return true;
}

bool UJGBattleSubsystem::InitializeActiveBattle(FJGBattleCommandResult& OutResult)
{
	OutResult = FJGBattleCommandResult{};

	if (!bHasPendingBattleStart || !PendingBattleStartRequest.IsValidRequest())
	{
		OutResult.FailureReason = EJGBattleFailureReason::InvalidState;
		return false;
	}

	FRandomStream Stream(PendingBattleStartRequest.Seed);
	FJGBattleState NewState;
	if (!FJGBattleSimulation::InitializeFromEncounter(PendingBattleStartRequest.EncounterDefinition, Stream, NewState, OutResult))
	{
		return false;
	}

	ActiveBattleStartRequest = PendingBattleStartRequest;
	ActiveBattleState = NewState;
	ActiveBattleStreamAdvance = 0;
	bHasActiveBattle = true;
	ClearPendingBattleStart();

	return true;
}

bool UJGBattleSubsystem::ExecuteActiveBattleCommand(
	const FJGBattleCommand& Command,
	FJGBattleCommandResult& OutResult,
	int32& OutStreamAdvance)
{
	OutResult = FJGBattleCommandResult{};
	OutStreamAdvance = ActiveBattleStreamAdvance;

	if (!bHasActiveBattle)
	{
		OutResult.FailureReason = EJGBattleFailureReason::InvalidState;
		return false;
	}

	const bool bSuccess = UJGBattleSimulationBlueprintLibrary::ExecuteBattleCommand(
		ActiveBattleState,
		Command,
		ActiveBattleStartRequest.Seed,
		ActiveBattleStreamAdvance,
		OutResult,
		OutStreamAdvance);

	ActiveBattleStreamAdvance = OutStreamAdvance;
	return bSuccess;
}

bool UJGBattleSubsystem::RunEnemyTurnsUntilPlayerInputNeeded(FJGBattleCommandResult& OutResult, int32& OutStreamAdvance)
{
	OutResult = FJGBattleCommandResult{};
	OutStreamAdvance = ActiveBattleStreamAdvance;

	if (!bHasActiveBattle)
	{
		OutResult.FailureReason = EJGBattleFailureReason::InvalidState;
		return false;
	}

	const bool bSuccess = UJGBattleSimulationBlueprintLibrary::RunEnemyTurnsUntilPlayerInputNeeded(
		ActiveBattleState,
		ActiveBattleStartRequest.Seed,
		ActiveBattleStreamAdvance,
		OutResult,
		OutStreamAdvance);

	ActiveBattleStreamAdvance = OutStreamAdvance;
	return bSuccess;
}

bool UJGBattleSubsystem::EndActiveBattle(const EJGBattleTeam WinningTeam, FJGBattleStartRequest& OutEndedRequest)
{
	OutEndedRequest = FJGBattleStartRequest{};

	if (!bHasActiveBattle)
	{
		return false;
	}

	LastWinningTeam = WinningTeam;
	LastCompletedBattleStartRequest = ActiveBattleStartRequest;
	OutEndedRequest = ActiveBattleStartRequest;
	bHasCompletedBattle = true;

	ClearActiveBattle();
	return true;
}

void UJGBattleSubsystem::ClearPendingBattleStart()
{
	bHasPendingBattleStart = false;
	PendingBattleStartRequest = FJGBattleStartRequest{};
}

void UJGBattleSubsystem::ClearActiveBattle()
{
	bHasActiveBattle = false;
	ActiveBattleStartRequest = FJGBattleStartRequest{};
	ActiveBattleState = FJGBattleState{};
	ActiveBattleStreamAdvance = 0;
}

bool UJGBattleSubsystem::HasPendingBattleStart() const
{
	return bHasPendingBattleStart;
}

bool UJGBattleSubsystem::HasActiveBattle() const
{
	return bHasActiveBattle;
}

FJGBattleStartRequest UJGBattleSubsystem::GetPendingBattleStartRequest() const
{
	return PendingBattleStartRequest;
}

FJGBattleStartRequest UJGBattleSubsystem::GetActiveBattleStartRequest() const
{
	return ActiveBattleStartRequest;
}

FJGBattleState UJGBattleSubsystem::GetActiveBattleState() const
{
	return ActiveBattleState;
}

int32 UJGBattleSubsystem::GetActiveBattleStreamAdvance() const
{
	return ActiveBattleStreamAdvance;
}

bool UJGBattleSubsystem::HasCompletedBattle() const
{
	return bHasCompletedBattle;
}

EJGBattleTeam UJGBattleSubsystem::GetLastWinningTeam() const
{
	return LastWinningTeam;
}

FJGBattleStartRequest UJGBattleSubsystem::GetLastCompletedBattleStartRequest() const
{
	return LastCompletedBattleStartRequest;
}
