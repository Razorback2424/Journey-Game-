#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/JGGridTypes.h"
#include "JGGridBlueprintLibrary.generated.h"

UCLASS()
class JOURNEYGAME_API UJGGridBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "JourneyGame|Grid", meta = (AdvancedDisplay = "TileSizeUU,bPreserveInputZ,OverrideZ"))
	static FVector SnapWorldLocationToGrid(
		FVector WorldLocation,
		float TileSizeUU = 32.0f,
		bool bPreserveInputZ = true,
		float OverrideZ = 0.0f);

	UFUNCTION(BlueprintPure, Category = "JourneyGame|Grid", meta = (AdvancedDisplay = "TileSizeUU"))
	static FJGGridCoord GridCoordFromWorld(FVector WorldLocation, float TileSizeUU = 32.0f);

	UFUNCTION(BlueprintPure, Category = "JourneyGame|Grid", meta = (AdvancedDisplay = "TileSizeUU,Z"))
	static FVector WorldFromGridCoord(FJGGridCoord GridCoord, float TileSizeUU = 32.0f, float Z = 0.0f);

	UFUNCTION(BlueprintPure, Category = "JourneyGame|Grid")
	static FVector2D DirectionToGridStep(FVector2D InputDirection, bool bPreferXOnTie = true);

	UFUNCTION(BlueprintPure, Category = "JourneyGame|Grid")
	static bool IsWorldLocationOnGrid(FVector WorldLocation, float TileSizeUU = 32.0f, float Tolerance = 0.01f);

	UFUNCTION(BlueprintPure, Category = "JourneyGame|Grid")
	static FVector GridStepToWorldDelta(FVector2D GridStepDirection, float TileSizeUU = 32.0f);

	UFUNCTION(BlueprintPure, Category = "JourneyGame|Grid")
	static FVector ForwardVectorFromGridStep(FVector2D GridStepDirection);
};
