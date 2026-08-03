#include "Battle/JGBattlePlayerControllerBase.h"

#include "Battle/JGBattleSubsystem.h"
#include "Engine/GameInstance.h"

AJGBattlePlayerControllerBase::AJGBattlePlayerControllerBase()
{
	bShowMouseCursor = true;
}

bool AJGBattlePlayerControllerBase::ExecuteBattleCommand(const FJGBattleCommand& Command, FJGBattleCommandResult& OutResult)
{
	OutResult = FJGBattleCommandResult{};

	UJGBattleSubsystem* BattleSubsystem = ResolveBattleSubsystem();
	if (!BattleSubsystem)
	{
		OutResult.FailureReason = EJGBattleFailureReason::InvalidState;
		return false;
	}

	int32 StreamAdvance = 0;
	const bool bSuccess = BattleSubsystem->ExecuteActiveBattleCommand(Command, OutResult, StreamAdvance);
	BP_OnBattleCommandExecuted(Command, OutResult);
	return bSuccess;
}

bool AJGBattlePlayerControllerBase::SelectCurrentUnit(FJGBattleCommandResult& OutResult)
{
	OutResult = FJGBattleCommandResult{};

	const UJGBattleSubsystem* BattleSubsystem = ResolveBattleSubsystem();
	if (!BattleSubsystem || !BattleSubsystem->HasActiveBattle())
	{
		OutResult.FailureReason = EJGBattleFailureReason::InvalidState;
		return false;
	}

	const FJGBattleState& State = BattleSubsystem->GetActiveBattleState();
	if (!State.TurnOrder.IsValidIndex(State.CurrentTurnIndex))
	{
		OutResult.FailureReason = EJGBattleFailureReason::InvalidState;
		return false;
	}

	FJGBattleCommand Command;
	Command.Type = EJGBattleCommandType::SelectUnit;
	Command.UnitHandle = State.TurnOrder[State.CurrentTurnIndex];
	return ExecuteBattleCommand(Command, OutResult);
}

bool AJGBattlePlayerControllerBase::MoveSelectedUnitTo(const FJGBattleGridCoord TargetCoord, FJGBattleCommandResult& OutResult)
{
	FJGBattleCommand Command;
	Command.Type = EJGBattleCommandType::Move;
	Command.TargetCoord = TargetCoord;
	return ExecuteBattleCommand(Command, OutResult);
}

bool AJGBattlePlayerControllerBase::ConfirmMoveSelectedUnitTo(const FJGBattleGridCoord TargetCoord, FJGBattleCommandResult& OutResult)
{
	FJGBattleCommand Command;
	Command.Type = EJGBattleCommandType::ConfirmMove;
	Command.TargetCoord = TargetCoord;
	return ExecuteBattleCommand(Command, OutResult);
}

bool AJGBattlePlayerControllerBase::BeginAttack(FJGBattleCommandResult& OutResult)
{
	FJGBattleCommand Command;
	Command.Type = EJGBattleCommandType::BeginAttack;
	return ExecuteBattleCommand(Command, OutResult);
}

bool AJGBattlePlayerControllerBase::AttackUnit(const FJGBattleUnitHandle TargetUnitHandle, FJGBattleCommandResult& OutResult)
{
	FJGBattleCommand Command;
	Command.Type = EJGBattleCommandType::Attack;
	Command.TargetUnitHandle = TargetUnitHandle;
	return ExecuteBattleCommand(Command, OutResult);
}

bool AJGBattlePlayerControllerBase::EndTurn(FJGBattleCommandResult& OutResult)
{
	FJGBattleCommand Command;
	Command.Type = EJGBattleCommandType::EndTurn;
	return ExecuteBattleCommand(Command, OutResult);
}

bool AJGBattlePlayerControllerBase::RunEnemyTurnsUntilPlayerInputNeeded(FJGBattleCommandResult& OutResult)
{
	OutResult = FJGBattleCommandResult{};

	UJGBattleSubsystem* BattleSubsystem = ResolveBattleSubsystem();
	if (!BattleSubsystem)
	{
		OutResult.FailureReason = EJGBattleFailureReason::InvalidState;
		return false;
	}

	int32 StreamAdvance = 0;
	const bool bSuccess = BattleSubsystem->RunEnemyTurnsUntilPlayerInputNeeded(OutResult, StreamAdvance);
	BP_OnEnemyTurnsResolved(OutResult);
	return bSuccess;
}

UJGBattleSubsystem* AJGBattlePlayerControllerBase::ResolveBattleSubsystem() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UJGBattleSubsystem>();
	}
	return nullptr;
}
