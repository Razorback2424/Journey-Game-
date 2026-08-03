#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Encounters/JGEncounterTypes.h"
#include "JGEncounterBlueprintLibrary.generated.h"

class UJGEncounterTableDataAsset;

UCLASS()
class JOURNEYGAME_API UJGEncounterBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "JourneyGame|Encounter")
	static int32 GetEncounterTableTotalWeight(const UJGEncounterTableDataAsset* EncounterTable);

	UFUNCTION(BlueprintCallable, Category = "JourneyGame|Encounter")
	static bool TryPickEncounterByRoll(
		const UJGEncounterTableDataAsset* EncounterTable,
		int32 RollValue,
		FJGEncounterEntry& OutEntry);

	UFUNCTION(BlueprintCallable, Category = "JourneyGame|Encounter")
	static bool TryPickEncounterRandom(
		const UJGEncounterTableDataAsset* EncounterTable,
		FJGEncounterEntry& OutEntry,
		int32& OutRollValue);

	UFUNCTION(BlueprintCallable, Category = "JourneyGame|Encounter")
	static bool TryPickEncounterFromStream(
		const UJGEncounterTableDataAsset* EncounterTable,
		UPARAM(ref) FRandomStream& Stream,
		FJGEncounterEntry& OutEntry,
		int32& OutRollValue);
};

