#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Battle/JGBattleTypes.h"
#include "JGBattlePlayerControllerBase.generated.h"

class UJGBattleSubsystem;

UCLASS(Blueprintable)
class JOURNEYGAME_API AJGBattlePlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	AJGBattlePlayerControllerBase();

	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool ExecuteBattleCommand(const FJGBattleCommand& Command, FJGBattleCommandResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool SelectCurrentUnit(FJGBattleCommandResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool MoveSelectedUnitTo(FJGBattleGridCoord TargetCoord, FJGBattleCommandResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool ConfirmMoveSelectedUnitTo(FJGBattleGridCoord TargetCoord, FJGBattleCommandResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool BeginAttack(FJGBattleCommandResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool AttackUnit(FJGBattleUnitHandle TargetUnitHandle, FJGBattleCommandResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool EndTurn(FJGBattleCommandResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool RunEnemyTurnsUntilPlayerInputNeeded(FJGBattleCommandResult& OutResult);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Battle", meta = (DisplayName = "On Battle Command Executed"))
	void BP_OnBattleCommandExecuted(const FJGBattleCommand& Command, const FJGBattleCommandResult& Result);

	UFUNCTION(BlueprintImplementableEvent, Category = "Battle", meta = (DisplayName = "On Enemy Turns Resolved"))
	void BP_OnEnemyTurnsResolved(const FJGBattleCommandResult& Result);

private:
	UJGBattleSubsystem* ResolveBattleSubsystem() const;
};
