#include "Overworld/JGOverworldGameModeBase.h"

#include "Battle/JGBattleSubsystem.h"
#include "Battle/Data/JGBattleEncounterDefinitionDataAsset.h"
#include "Core/JGLog.h"
#include "Encounters/JGEncounterBlueprintLibrary.h"
#include "Encounters/JGEncounterTableDataAsset.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Overworld/JGOverworldCharacterBase.h"
#include "Overworld/JGOverworldPlayerControllerBase.h"
#include "Overworld/JGOverworldSubsystem.h"
#include "UObject/ConstructorHelpers.h"

AJGOverworldGameModeBase::AJGOverworldGameModeBase()
{
	DefaultPawnClass = AJGOverworldCharacterBase::StaticClass();
	PlayerControllerClass = AJGOverworldPlayerControllerBase::StaticClass();

	if (static ConstructorHelpers::FClassFinder<APawn> OverworldPlayerClass(TEXT("/Game/Overworld/Blueprints/Player/BP_OverworldPlayer")); OverworldPlayerClass.Succeeded()
		&& OverworldPlayerClass.Class->IsChildOf(AJGOverworldCharacterBase::StaticClass()))
	{
		DefaultPawnClass = OverworldPlayerClass.Class;
	}

	if (static ConstructorHelpers::FClassFinder<APlayerController> OverworldControllerClass(TEXT("/Game/Overworld/Blueprints/Player/BP_OverworldController")); OverworldControllerClass.Succeeded()
		&& OverworldControllerClass.Class->IsChildOf(AJGOverworldPlayerControllerBase::StaticClass()))
	{
		PlayerControllerClass = OverworldControllerClass.Class;
	}
}

void AJGOverworldGameModeBase::StartPlay()
{
	Super::StartPlay();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UJGOverworldSubsystem* OverworldSubsystem = GameInstance->GetSubsystem<UJGOverworldSubsystem>())
		{
			if (bAutoSetCurrentMapIdFromWorld && GetWorld())
			{
				OverworldSubsystem->SetCurrentMapId(FName(*UGameplayStatics::GetCurrentLevelName(this, true)));
			}
		}
	}

	if (bAutoProcessPendingMapTransition)
	{
		TryProcessPendingMapTransition();
	}

	if (bAutoProcessPendingEncounterForBattle)
	{
		FJGBattleCommandResult Result;
		TryProcessPendingEncounterForBattle(Result);
	}

	UE_LOG(LogJourneyOverworld, Verbose, TEXT("Overworld GameMode started: %s"), *GetName());
}

bool AJGOverworldGameModeBase::TryConsumePendingMapTransition(FJGMapTransitionRequest& OutRequest)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UJGOverworldSubsystem* OverworldSubsystem = GameInstance->GetSubsystem<UJGOverworldSubsystem>())
		{
			if (OverworldSubsystem->ConsumePendingMapTransition(OutRequest))
			{
				BP_OnPendingMapTransitionConsumed(OutRequest);
				return true;
			}
		}
	}

	return false;
}

bool AJGOverworldGameModeBase::TryProcessPendingMapTransition()
{
	FJGMapTransitionRequest PendingRequest;
	if (!TryConsumePendingMapTransition(PendingRequest))
	{
		return false;
	}

	if (bOpenLevelWhenProcessingPendingTransition)
	{
		if (PendingRequest.bSeamless)
		{
			UE_LOG(LogJourneyOverworld, Verbose, TEXT("Pending transition requested seamless travel; OpenLevel fallback is non-seamless"));
		}

		UGameplayStatics::OpenLevel(this, PendingRequest.MapId);
	}

	return true;
}

bool AJGOverworldGameModeBase::TryProcessPendingEncounterForBattle(FJGBattleCommandResult& OutResult)
{
	OutResult = FJGBattleCommandResult{};

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return TryProcessPendingEncounterForBattleWithSubsystems(
			GameInstance->GetSubsystem<UJGOverworldSubsystem>(),
			GameInstance->GetSubsystem<UJGBattleSubsystem>(),
			OutResult);
	}

	OutResult.FailureReason = EJGBattleFailureReason::InvalidState;
	return false;
}

bool AJGOverworldGameModeBase::TryProcessPendingEncounterForBattleWithSubsystems(
	UJGOverworldSubsystem* OverworldSubsystem,
	UJGBattleSubsystem* BattleSubsystem,
	FJGBattleCommandResult& OutResult)
{
	OutResult = FJGBattleCommandResult{};

	if (!OverworldSubsystem || !BattleSubsystem)
	{
		OutResult.FailureReason = EJGBattleFailureReason::InvalidState;
		UE_LOG(LogJourneyEncounter, Warning, TEXT("Cannot process pending encounter: missing subsystem"));
		return false;
	}

	FJGEncounterRequest EncounterRequest;
	if (!OverworldSubsystem->ConsumePendingEncounter(EncounterRequest))
	{
		return false;
	}

	FJGEncounterEntry PickedEntry;
	int32 RollValue = INDEX_NONE;
	if (!UJGEncounterBlueprintLibrary::TryPickEncounterRandom(EncounterRequest.EncounterTable, PickedEntry, RollValue))
	{
		OutResult.FailureReason = EJGBattleFailureReason::InvalidState;
		UE_LOG(LogJourneyEncounter, Warning, TEXT("Pending encounter could not pick from table %s"),
			*GetNameSafe(EncounterRequest.EncounterTable));
		return false;
	}

	UPrimaryDataAsset* Payload = PickedEntry.BattlePayload.Get();
	if (!Payload)
	{
		Payload = PickedEntry.BattlePayload.LoadSynchronous();
	}

	UJGBattleEncounterDefinitionDataAsset* BattleDefinition = Cast<UJGBattleEncounterDefinitionDataAsset>(Payload);
	if (!BattleDefinition)
	{
		OutResult.FailureReason = EJGBattleFailureReason::InvalidState;
		UE_LOG(LogJourneyEncounter, Warning, TEXT("Encounter '%s' has no battle definition payload"),
			*PickedEntry.EncounterId.ToString());
		return false;
	}

	FJGBattleStartRequest BattleStartRequest;
	BattleStartRequest.EncounterDefinition = BattleDefinition;
	BattleStartRequest.Seed = BattleDefinition->bUseFixedSeed ? BattleDefinition->FixedSeed : FMath::Rand();
	BattleStartRequest.SourceEncounterId = PickedEntry.EncounterId;
	BattleStartRequest.ReturnMapId = OverworldSubsystem->GetCurrentMapId();
	BattleStartRequest.ReturnSpawnTag = OverworldSubsystem->GetNextSpawnTag();

	if (!BattleSubsystem->RequestBattleStart(BattleStartRequest, OutResult))
	{
		return false;
	}

	if (bOpenBattleLevelWhenProcessingPendingEncounter && !BattleLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, BattleLevelName);
	}

	UE_LOG(LogJourneyEncounter, Log, TEXT("Encounter '%s' queued battle '%s' (Roll=%d)"),
		*PickedEntry.EncounterId.ToString(),
		*GetNameSafe(BattleDefinition),
		RollValue);

	return true;
}
