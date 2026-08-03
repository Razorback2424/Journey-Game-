#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Encounters/JGEncounterTypes.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif
#include "JGEncounterTableDataAsset.generated.h"

UCLASS(BlueprintType)
class JOURNEYGAME_API UJGEncounterTableDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter")
	FName TableId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter", meta = (TitleProperty = "EncounterId"))
	TArray<FJGEncounterEntry> Entries;

	UFUNCTION(BlueprintCallable, Category = "Encounter|Validation")
	bool ValidateEncounterTable(FString& OutError) const;

	UFUNCTION(BlueprintPure, Category = "Encounter|Validation")
	int32 GetTotalEncounterWeight() const;

	UFUNCTION(BlueprintPure, Category = "Encounter|Validation")
	bool HasValidEntries() const;

	bool ValidateEncounterTableInternal(FString* OutError, bool bLogWarnings) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
