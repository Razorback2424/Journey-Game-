#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Battle/JGBattleTypes.h"
#include "JGBattleGameModeBase.generated.h"

class UJGBattleSubsystem;

UCLASS(Blueprintable)
class JOURNEYGAME_API AJGBattleGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AJGBattleGameModeBase();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	bool bInitializePendingBattleOnStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	bool bRunEnemyTurnsAfterInitialization = true;

	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool TryInitializePendingBattle(FJGBattleCommandResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool TryRunEnemyTurnsUntilPlayerInputNeeded(FJGBattleCommandResult& OutResult, int32& OutStreamAdvance);

protected:
	virtual void StartPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Battle", meta = (DisplayName = "On Battle Initialized"))
	void BP_OnBattleInitialized(const FJGBattleCommandResult& Result);

	UFUNCTION(BlueprintImplementableEvent, Category = "Battle", meta = (DisplayName = "On Enemy Turns Resolved"))
	void BP_OnEnemyTurnsResolved(const FJGBattleCommandResult& Result);

private:
	UJGBattleSubsystem* ResolveBattleSubsystem() const;
};
