#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "JGDebugBlueprintLibrary.generated.h"

UCLASS()
class JOURNEYGAME_API UJGDebugBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "JourneyGame|Debug", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "Color,Duration,bLogToOutput"))
	static void PrintJourneyDebugMessage(
		const UObject* WorldContextObject,
		const FString& Message,
		FLinearColor Color = FLinearColor::Yellow,
		float Duration = 2.0f,
		bool bLogToOutput = true);

	UFUNCTION(BlueprintCallable, Category = "JourneyGame|Debug", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "Duration,Thickness"))
	static void DrawJourneyDebugLine(
		const UObject* WorldContextObject,
		FVector Start,
		FVector End,
		FLinearColor Color = FLinearColor::Green,
		float Duration = 1.0f,
		float Thickness = 1.0f);
};

