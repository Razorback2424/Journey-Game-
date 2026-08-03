#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Battle/JGBattleTypes.h"
#include "JGBattleSimulation.generated.h"

class UJGBattleEncounterDefinitionDataAsset;

UCLASS()
class JOURNEYGAME_API UJGBattleSimulationBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "JourneyGame|Battle")
	static bool InitializeBattleState(
		UJGBattleEncounterDefinitionDataAsset* EncounterDefinition,
		int32 Seed,
		FJGBattleState& OutState,
		FJGBattleCommandResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "JourneyGame|Battle")
	static bool ExecuteBattleCommand(
		UPARAM(ref) FJGBattleState& State,
		const FJGBattleCommand& Command,
		int32 Seed,
		int32 StreamAdvance,
		FJGBattleCommandResult& OutResult,
		int32& OutStreamAdvance);

	UFUNCTION(BlueprintCallable, Category = "JourneyGame|Battle")
	static bool RunEnemyTurnsUntilPlayerInputNeeded(
		UPARAM(ref) FJGBattleState& State,
		int32 Seed,
		int32 StreamAdvance,
		FJGBattleCommandResult& OutResult,
		int32& OutStreamAdvance);

	UFUNCTION(BlueprintPure, Category = "JourneyGame|Battle")
	static FJGBattleDebugSnapshot BuildDebugSnapshot(const FJGBattleState& State);
};

struct JOURNEYGAME_API FJGBattleSimulation
{
	static bool InitializeFromEncounter(UJGBattleEncounterDefinitionDataAsset* EncounterDefinition, FRandomStream& Stream, FJGBattleState& OutState, FJGBattleCommandResult& OutResult);
	static FJGBattleCommandResult ExecuteCommand(FJGBattleState& State, const FJGBattleCommand& Command, FRandomStream& Stream);
	static FJGBattleCommandResult RunEnemyTurnsUntilPlayerInputNeeded(FJGBattleState& State, FRandomStream& Stream);
};
