#include "Core/JGDeveloperSettings.h"

UJGDeveloperSettings::UJGDeveloperSettings()
{
	DefaultGridSettings.TileSizeUU = 32.0f;
	DefaultGridSettings.bSnapOnBeginPlay = true;
	DefaultGridSettings.StepDuration = 0.14f;
}

FName UJGDeveloperSettings::GetCategoryName() const
{
	return TEXT("Game");
}

const UJGDeveloperSettings* UJGDeveloperSettings::Get()
{
	return GetDefault<UJGDeveloperSettings>();
}

