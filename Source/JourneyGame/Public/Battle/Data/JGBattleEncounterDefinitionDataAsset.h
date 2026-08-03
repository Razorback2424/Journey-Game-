#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Battle/JGBattleTypes.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif
#include "JGBattleEncounterDefinitionDataAsset.generated.h"

class UJGBattleUnitArchetypeDataAsset;

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGBattleEncounterSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	TObjectPtr<UJGBattleUnitArchetypeDataAsset> Archetype = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FJGBattleGridCoord SpawnCoord;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 LevelOverride = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FName UnitIdOverride = NAME_None;
};

UCLASS(BlueprintType)
class JOURNEYGAME_API UJGBattleEncounterDefinitionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	FName BattleId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle", meta = (ClampMin = "3"))
	int32 GridWidth = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle", meta = (ClampMin = "3"))
	int32 GridHeight = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	TArray<FJGBattleGridCoord> Blockers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle", meta = (TitleProperty = "SpawnCoord"))
	TArray<FJGBattleEncounterSlot> PlayerSlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle", meta = (TitleProperty = "SpawnCoord"))
	TArray<FJGBattleEncounterSlot> EnemySlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	bool bUseFixedSeed = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	int32 FixedSeed = 1337;

	UFUNCTION(BlueprintCallable, Category = "Battle|Validation")
	bool ValidateDefinition(FString& OutError) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};

