#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Encounters/JGEncounterTypes.h"
#include "Overworld/JGOverworldTypes.h"
#include "JGOverworldSubsystem.generated.h"

UCLASS(BlueprintType)
class JOURNEYGAME_API UJGOverworldSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Overworld")
	void SetCurrentMapId(FName InMapId);

	UFUNCTION(BlueprintPure, Category = "Overworld")
	FName GetCurrentMapId() const;

	UFUNCTION(BlueprintCallable, Category = "Overworld")
	void SetNextSpawnTag(FName InSpawnTag);

	UFUNCTION(BlueprintPure, Category = "Overworld")
	FName GetNextSpawnTag() const;

	UFUNCTION(BlueprintCallable, Category = "Overworld|Encounter")
	void SetEncountersEnabled(bool bInEnabled);

	UFUNCTION(BlueprintPure, Category = "Overworld|Encounter")
	bool AreEncountersEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "Overworld|Encounter")
	bool RequestEncounter(const FJGEncounterRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "Overworld|Encounter")
	bool ConsumePendingEncounter(FJGEncounterRequest& OutRequest);

	UFUNCTION(BlueprintCallable, Category = "Overworld|Encounter")
	void ClearPendingEncounter();

	UFUNCTION(BlueprintPure, Category = "Overworld|Encounter")
	bool HasPendingEncounter() const;

	UFUNCTION(BlueprintCallable, Category = "Overworld|Transition")
	bool RequestMapTransition(const FJGMapTransitionRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "Overworld|Transition")
	bool ConsumePendingMapTransition(FJGMapTransitionRequest& OutRequest);

	UFUNCTION(BlueprintCallable, Category = "Overworld|Transition")
	void ClearPendingMapTransition();

	UFUNCTION(BlueprintPure, Category = "Overworld|Transition")
	bool HasPendingMapTransition() const;

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Overworld")
	FName CurrentMapId = NAME_None;

	UPROPERTY(VisibleInstanceOnly, Category = "Overworld")
	FName NextSpawnTag = NAME_None;

	UPROPERTY(VisibleInstanceOnly, Category = "Overworld|Encounter")
	bool bEncountersEnabled = true;

	UPROPERTY(VisibleInstanceOnly, Category = "Overworld|Encounter")
	bool bHasPendingEncounter = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Overworld|Encounter")
	FJGEncounterRequest PendingEncounterRequest;

	UPROPERTY(VisibleInstanceOnly, Category = "Overworld|Transition")
	bool bHasPendingMapTransition = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Overworld|Transition")
	FJGMapTransitionRequest PendingMapTransitionRequest;
};
