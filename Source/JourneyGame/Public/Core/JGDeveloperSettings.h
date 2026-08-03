#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Data/JGGridTypes.h"
#include "JGDeveloperSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Journey Game"))
class JOURNEYGAME_API UJGDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UJGDeveloperSettings();

	virtual FName GetCategoryName() const override;

	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Overworld|Grid")
	FJGGridSettings DefaultGridSettings;

	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Overworld|Interaction", meta = (ClampMin = "1.0"))
	float DefaultInteractionTraceDistance = 32.0f;

	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Overworld|Encounter")
	bool bEncountersEnabledByDefault = true;

	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Debug")
	bool bEnableVerboseOverworldLogs = false;

	UFUNCTION(BlueprintPure, Category = "JourneyGame|Settings")
	static const UJGDeveloperSettings* Get();
};

