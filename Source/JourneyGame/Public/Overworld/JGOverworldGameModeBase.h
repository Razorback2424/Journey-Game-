#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Battle/JGBattleTypes.h"
#include "Overworld/JGOverworldTypes.h"
#include "JGOverworldGameModeBase.generated.h"

class UJGBattleSubsystem;
class UJGOverworldSubsystem;

UCLASS(Blueprintable)
class JOURNEYGAME_API AJGOverworldGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AJGOverworldGameModeBase();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overworld|Transition")
	bool bAutoSetCurrentMapIdFromWorld = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overworld|Transition")
	bool bAutoProcessPendingMapTransition = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overworld|Transition")
	bool bOpenLevelWhenProcessingPendingTransition = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overworld|Encounter")
	bool bAutoProcessPendingEncounterForBattle = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overworld|Encounter")
	bool bOpenBattleLevelWhenProcessingPendingEncounter = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overworld|Encounter")
	FName BattleLevelName = TEXT("Lvl_Battle_Test");

	UFUNCTION(BlueprintCallable, Category = "Overworld|Transition")
	bool TryConsumePendingMapTransition(FJGMapTransitionRequest& OutRequest);

	UFUNCTION(BlueprintCallable, Category = "Overworld|Transition")
	bool TryProcessPendingMapTransition();

	UFUNCTION(BlueprintCallable, Category = "Overworld|Encounter")
	bool TryProcessPendingEncounterForBattle(FJGBattleCommandResult& OutResult);

	bool TryProcessPendingEncounterForBattleWithSubsystems(
		UJGOverworldSubsystem* OverworldSubsystem,
		UJGBattleSubsystem* BattleSubsystem,
		FJGBattleCommandResult& OutResult);

protected:
	virtual void StartPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Overworld|Transition", meta = (DisplayName = "On Pending Map Transition Consumed"))
	void BP_OnPendingMapTransitionConsumed(const FJGMapTransitionRequest& Request);
};
