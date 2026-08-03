#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "JGInteractionBlueprintLibrary.generated.h"

class AActor;

UCLASS()
class JOURNEYGAME_API UJGInteractionBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "JourneyGame|Interaction")
	static bool ImplementsInteractable(const AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "JourneyGame|Interaction")
	static bool CanInteractWithActor(const AActor* TargetActor, AActor* InteractingActor);

	UFUNCTION(BlueprintCallable, Category = "JourneyGame|Interaction")
	static bool TryInteractWithActor(AActor* TargetActor, AActor* InteractingActor);

	UFUNCTION(BlueprintPure, Category = "JourneyGame|Interaction")
	static FText GetInteractionPromptOrDefault(const AActor* TargetActor, FText DefaultPrompt);

	UFUNCTION(BlueprintCallable, Category = "JourneyGame|Interaction", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "ActorsToIgnore"))
	static bool TraceInteractableLine(
		const UObject* WorldContextObject,
		FVector Start,
		FVector End,
		TEnumAsByte<ECollisionChannel> TraceChannel,
		const TArray<AActor*>& ActorsToIgnore,
		bool bDrawDebug,
		FHitResult& OutHit,
		AActor*& OutInteractableActor);

	UFUNCTION(BlueprintCallable, Category = "JourneyGame|Interaction", meta = (WorldContext = "WorldContextObject"))
	static bool TraceInteractableFromActorForward(
		const UObject* WorldContextObject,
		AActor* SourceActor,
		float Distance,
		TEnumAsByte<ECollisionChannel> TraceChannel,
		bool bDrawDebug,
		FHitResult& OutHit,
		AActor*& OutInteractableActor);
};

