#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "JGMapEncounterProfileDataAsset.generated.h"

class UJGEncounterTableDataAsset;

UCLASS(BlueprintType)
class JOURNEYGAME_API UJGMapEncounterProfileDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overworld")
	FName MapId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overworld")
	bool bEncountersEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overworld", meta = (ClampMin = "1"))
	int32 StepsPerEncounterCheck = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overworld", meta = (ClampMin = "0"))
	int32 EncounterStepCooldown = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overworld")
	TObjectPtr<UJGEncounterTableDataAsset> DefaultEncounterTable = nullptr;
};

