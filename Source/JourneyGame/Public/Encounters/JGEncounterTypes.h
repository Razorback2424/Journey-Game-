#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "JGEncounterTypes.generated.h"

class AActor;
class UJGEncounterTableDataAsset;
class UPrimaryDataAsset;

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGEncounterEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	FName EncounterId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter", meta = (ClampMin = "1"))
	int32 Weight = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter", meta = (ClampMin = "1"))
	int32 MinLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter", meta = (ClampMin = "1"))
	int32 MaxLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	TSoftObjectPtr<UPrimaryDataAsset> BattlePayload;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	FGameplayTagContainer Tags;
};

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGEncounterRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	TObjectPtr<AActor> TriggeringActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	TObjectPtr<UJGEncounterTableDataAsset> EncounterTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	int32 TriggerStepIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	float RequestedAtSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	FGameplayTagContainer ContextTags;

	bool HasMinimumData() const
	{
		return EncounterTable != nullptr;
	}
};
