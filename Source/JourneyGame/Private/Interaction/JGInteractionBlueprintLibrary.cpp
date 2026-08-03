#include "Interaction/JGInteractionBlueprintLibrary.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Interaction/JGInteractableInterface.h"

bool UJGInteractionBlueprintLibrary::ImplementsInteractable(const AActor* Actor)
{
	return IsValid(Actor) && Actor->GetClass()->ImplementsInterface(UJGInteractableInterface::StaticClass());
}

bool UJGInteractionBlueprintLibrary::CanInteractWithActor(const AActor* TargetActor, AActor* InteractingActor)
{
	if (!ImplementsInteractable(TargetActor))
	{
		return false;
	}

	return IJGInteractableInterface::Execute_CanInteract(const_cast<AActor*>(TargetActor), InteractingActor);
}

bool UJGInteractionBlueprintLibrary::TryInteractWithActor(AActor* TargetActor, AActor* InteractingActor)
{
	if (!CanInteractWithActor(TargetActor, InteractingActor))
	{
		return false;
	}

	IJGInteractableInterface::Execute_Interact(TargetActor, InteractingActor);
	return true;
}

FText UJGInteractionBlueprintLibrary::GetInteractionPromptOrDefault(const AActor* TargetActor, const FText DefaultPrompt)
{
	if (!ImplementsInteractable(TargetActor))
	{
		return DefaultPrompt;
	}

	const FText Prompt = IJGInteractableInterface::Execute_GetInteractionPrompt(const_cast<AActor*>(TargetActor));
	return Prompt.IsEmpty() ? DefaultPrompt : Prompt;
}

bool UJGInteractionBlueprintLibrary::TraceInteractableLine(
	const UObject* WorldContextObject,
	const FVector Start,
	const FVector End,
	const TEnumAsByte<ECollisionChannel> TraceChannel,
	const TArray<AActor*>& ActorsToIgnore,
	const bool bDrawDebug,
	FHitResult& OutHit,
	AActor*& OutInteractableActor)
{
	OutHit = FHitResult{};
	OutInteractableActor = nullptr;

	if (!WorldContextObject)
	{
		return false;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return false;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(JGInteractionTrace), false);
	Params.bReturnPhysicalMaterial = false;
	for (AActor* ActorToIgnore : ActorsToIgnore)
	{
		if (IsValid(ActorToIgnore))
		{
			Params.AddIgnoredActor(ActorToIgnore);
		}
	}

	const bool bHit = World->LineTraceSingleByChannel(OutHit, Start, End, TraceChannel, Params);
	if (bDrawDebug)
	{
		const FColor Color = bHit ? FColor::Green : FColor::Red;
		DrawDebugLine(World, Start, End, Color, false, 1.0f, 0, 1.5f);
		if (bHit)
		{
			DrawDebugPoint(World, OutHit.ImpactPoint, 8.0f, Color, false, 1.0f);
		}
	}

	if (!bHit)
	{
		return false;
	}

	AActor* HitActor = OutHit.GetActor();
	if (!ImplementsInteractable(HitActor))
	{
		return false;
	}

	OutInteractableActor = HitActor;
	return true;
}

bool UJGInteractionBlueprintLibrary::TraceInteractableFromActorForward(
	const UObject* WorldContextObject,
	AActor* SourceActor,
	const float Distance,
	const TEnumAsByte<ECollisionChannel> TraceChannel,
	const bool bDrawDebug,
	FHitResult& OutHit,
	AActor*& OutInteractableActor)
{
	if (!IsValid(SourceActor))
	{
		OutHit = FHitResult{};
		OutInteractableActor = nullptr;
		return false;
	}

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(SourceActor);

	const FVector Start = SourceActor->GetActorLocation();
	const FVector End = Start + (SourceActor->GetActorForwardVector() * Distance);
	return TraceInteractableLine(WorldContextObject, Start, End, TraceChannel, ActorsToIgnore, bDrawDebug, OutHit, OutInteractableActor);
}

