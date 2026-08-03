#include "Debug/JGDebugBlueprintLibrary.h"

#include "Core/JGLog.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

void UJGDebugBlueprintLibrary::PrintJourneyDebugMessage(
	const UObject* WorldContextObject,
	const FString& Message,
	const FLinearColor Color,
	const float Duration,
	const bool bLogToOutput)
{
	if (bLogToOutput)
	{
		UE_LOG(LogJourney, Log, TEXT("%s"), *Message);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color.ToFColor(true), Message);
	}
}

void UJGDebugBlueprintLibrary::DrawJourneyDebugLine(
	const UObject* WorldContextObject,
	const FVector Start,
	const FVector End,
	const FLinearColor Color,
	const float Duration,
	const float Thickness)
{
	if (!WorldContextObject)
	{
		return;
	}

	if (const UWorld* World = WorldContextObject->GetWorld())
	{
		DrawDebugLine(World, Start, End, Color.ToFColor(true), false, Duration, 0, Thickness);
	}
}

