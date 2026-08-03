#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Battle/JGBattleTypes.h"
#include "JGBattleSubsystem.generated.h"

class UJGBattleEncounterDefinitionDataAsset;

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGBattleStartRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	TObjectPtr<UJGBattleEncounterDefinitionDataAsset> EncounterDefinition = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 Seed = 1337;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FName SourceEncounterId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FName ReturnMapId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FName ReturnSpawnTag = NAME_None;

	bool IsValidRequest() const
	{
		return EncounterDefinition != nullptr;
	}
};

UCLASS(BlueprintType)
class JOURNEYGAME_API UJGBattleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool RequestBattleStart(const FJGBattleStartRequest& Request, FJGBattleCommandResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool InitializeActiveBattle(FJGBattleCommandResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool ExecuteActiveBattleCommand(const FJGBattleCommand& Command, FJGBattleCommandResult& OutResult, int32& OutStreamAdvance);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool RunEnemyTurnsUntilPlayerInputNeeded(FJGBattleCommandResult& OutResult, int32& OutStreamAdvance);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	bool EndActiveBattle(EJGBattleTeam WinningTeam, FJGBattleStartRequest& OutEndedRequest);

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void ClearPendingBattleStart();

	UFUNCTION(BlueprintCallable, Category = "Battle")
	void ClearActiveBattle();

	UFUNCTION(BlueprintPure, Category = "Battle")
	bool HasPendingBattleStart() const;

	UFUNCTION(BlueprintPure, Category = "Battle")
	bool HasActiveBattle() const;

	UFUNCTION(BlueprintPure, Category = "Battle")
	FJGBattleStartRequest GetPendingBattleStartRequest() const;

	UFUNCTION(BlueprintPure, Category = "Battle")
	FJGBattleStartRequest GetActiveBattleStartRequest() const;

	UFUNCTION(BlueprintPure, Category = "Battle")
	FJGBattleState GetActiveBattleState() const;

	UFUNCTION(BlueprintPure, Category = "Battle")
	int32 GetActiveBattleStreamAdvance() const;

	UFUNCTION(BlueprintPure, Category = "Battle")
	bool HasCompletedBattle() const;

	UFUNCTION(BlueprintPure, Category = "Battle")
	EJGBattleTeam GetLastWinningTeam() const;

	UFUNCTION(BlueprintPure, Category = "Battle")
	FJGBattleStartRequest GetLastCompletedBattleStartRequest() const;

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Battle")
	bool bHasPendingBattleStart = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle")
	FJGBattleStartRequest PendingBattleStartRequest;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle")
	bool bHasActiveBattle = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle")
	FJGBattleStartRequest ActiveBattleStartRequest;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle")
	FJGBattleState ActiveBattleState;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle")
	int32 ActiveBattleStreamAdvance = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle")
	bool bHasCompletedBattle = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle")
	EJGBattleTeam LastWinningTeam = EJGBattleTeam::Player;

	UPROPERTY(VisibleInstanceOnly, Category = "Battle")
	FJGBattleStartRequest LastCompletedBattleStartRequest;
};
