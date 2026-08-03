#include "Battle/JGBattleGameModeBase.h"

#include "Battle/JGBattleSubsystem.h"
#include "Battle/JGBattlePlayerControllerBase.h"
#include "Core/JGLog.h"
#include "Engine/GameInstance.h"

AJGBattleGameModeBase::AJGBattleGameModeBase()
{
	PlayerControllerClass = AJGBattlePlayerControllerBase::StaticClass();
	DefaultPawnClass = nullptr;
}

void AJGBattleGameModeBase::StartPlay()
{
	Super::StartPlay();

	if (bInitializePendingBattleOnStart)
	{
		FJGBattleCommandResult InitResult;
		if (TryInitializePendingBattle(InitResult) && bRunEnemyTurnsAfterInitialization)
		{
			FJGBattleCommandResult EnemyResult;
			int32 StreamAdvance = 0;
			TryRunEnemyTurnsUntilPlayerInputNeeded(EnemyResult, StreamAdvance);
		}
	}
}

bool AJGBattleGameModeBase::TryInitializePendingBattle(FJGBattleCommandResult& OutResult)
{
	OutResult = FJGBattleCommandResult{};

	UJGBattleSubsystem* BattleSubsystem = ResolveBattleSubsystem();
	if (!BattleSubsystem)
	{
		OutResult.FailureReason = EJGBattleFailureReason::InvalidState;
		return false;
	}

	const bool bInitialized = BattleSubsystem->InitializeActiveBattle(OutResult);
	if (bInitialized)
	{
		BP_OnBattleInitialized(OutResult);
		UE_LOG(LogJourney, Log, TEXT("Battle game mode initialized active battle"));
	}
	return bInitialized;
}

bool AJGBattleGameModeBase::TryRunEnemyTurnsUntilPlayerInputNeeded(FJGBattleCommandResult& OutResult, int32& OutStreamAdvance)
{
	OutResult = FJGBattleCommandResult{};
	OutStreamAdvance = 0;

	UJGBattleSubsystem* BattleSubsystem = ResolveBattleSubsystem();
	if (!BattleSubsystem)
	{
		OutResult.FailureReason = EJGBattleFailureReason::InvalidState;
		return false;
	}

	const bool bSuccess = BattleSubsystem->RunEnemyTurnsUntilPlayerInputNeeded(OutResult, OutStreamAdvance);
	if (bSuccess)
	{
		BP_OnEnemyTurnsResolved(OutResult);
	}
	return bSuccess;
}

UJGBattleSubsystem* AJGBattleGameModeBase::ResolveBattleSubsystem() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UJGBattleSubsystem>();
	}
	return nullptr;
}
