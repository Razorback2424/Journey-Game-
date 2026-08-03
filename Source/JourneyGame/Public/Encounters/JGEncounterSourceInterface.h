#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Encounters/JGEncounterTableDataAsset.h"
#include "JGEncounterSourceInterface.generated.h"

class AActor;

UINTERFACE(BlueprintType)
class JOURNEYGAME_API UJGEncounterSourceInterface : public UInterface
{
	GENERATED_BODY()
};

class JOURNEYGAME_API IJGEncounterSourceInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Encounter")
	UJGEncounterTableDataAsset* GetEncounterTable() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Encounter")
	bool CanTriggerEncounter(AActor* TriggeringActor) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Encounter")
	void HandleEncounterConsumed(AActor* TriggeringActor);
};
