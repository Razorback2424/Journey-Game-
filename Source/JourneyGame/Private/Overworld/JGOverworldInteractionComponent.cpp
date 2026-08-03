#include "Overworld/JGOverworldInteractionComponent.h"

#include "Core/JGDeveloperSettings.h"
#include "Core/JGLog.h"
#include "GameFramework/Controller.h"
#include "Interaction/JGInteractionBlueprintLibrary.h"

UJGOverworldInteractionComponent::UJGOverworldInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	DefaultPromptText = NSLOCTEXT("JourneyGame", "DefaultFocusedInteractionPrompt", "Interact");
}

void UJGOverworldInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (InteractionTraceDistance <= 0.0f)
	{
		if (const UJGDeveloperSettings* Settings = UJGDeveloperSettings::Get())
		{
			InteractionTraceDistance = Settings->DefaultInteractionTraceDistance;
		}
	}

	PrimaryComponentTick.SetTickFunctionEnable(bAutoRefreshFocus);
}

void UJGOverworldInteractionComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bAutoRefreshFocus)
	{
		RefreshInteractableFocus();
	}
}

AActor* UJGOverworldInteractionComponent::RefreshInteractableFocus()
{
	if (!bInteractionEnabled)
	{
		SetFocusedInteractable(nullptr);
		return nullptr;
	}

	AActor* SourceActor = ResolveInteractionSourceActor();
	if (!IsValid(SourceActor))
	{
		SetFocusedInteractable(nullptr);
		return nullptr;
	}

	FHitResult Hit;
	AActor* HitInteractable = nullptr;
	const bool bFoundInteractable = UJGInteractionBlueprintLibrary::TraceInteractableFromActorForward(
		this,
		SourceActor,
		InteractionTraceDistance,
		InteractionTraceChannel,
		bDrawDebugTrace,
		Hit,
		HitInteractable);

	if (!bFoundInteractable)
	{
		SetFocusedInteractable(nullptr);
		return nullptr;
	}

	UpdateFocusedInteractable(HitInteractable);
	return FocusedInteractableActor;
}

bool UJGOverworldInteractionComponent::UpdateFocusedInteractable(AActor* CandidateInteractable)
{
	AActor* SourceActor = ResolveInteractionSourceActor();
	if (!bInteractionEnabled || !IsValid(SourceActor) || !UJGInteractionBlueprintLibrary::CanInteractWithActor(CandidateInteractable, SourceActor))
	{
		SetFocusedInteractable(nullptr);
		return false;
	}

	SetFocusedInteractable(CandidateInteractable);
	return true;
}

bool UJGOverworldInteractionComponent::TryInteractFocused()
{
	return TryInteractActor(FocusedInteractableActor);
}

bool UJGOverworldInteractionComponent::TryInteractActor(AActor* TargetActor)
{
	AActor* SourceActor = ResolveInteractionSourceActor();
	const bool bSuccess = bInteractionEnabled
		&& IsValid(SourceActor)
		&& UJGInteractionBlueprintLibrary::TryInteractWithActor(TargetActor, SourceActor);

	if (!bSuccess)
	{
		UE_LOG(LogJourneyInteraction, Verbose, TEXT("Interaction attempt failed (source=%s, target=%s)"),
			*GetNameSafe(SourceActor),
			*GetNameSafe(TargetActor));
	}

	OnInteractionAttempted.Broadcast(TargetActor, bSuccess);
	return bSuccess;
}

AActor* UJGOverworldInteractionComponent::GetFocusedInteractable() const
{
	return FocusedInteractableActor;
}

bool UJGOverworldInteractionComponent::HasFocusedInteractable() const
{
	return IsValid(FocusedInteractableActor);
}

FText UJGOverworldInteractionComponent::GetFocusedPromptText() const
{
	return UJGInteractionBlueprintLibrary::GetInteractionPromptOrDefault(FocusedInteractableActor, DefaultPromptText);
}

AActor* UJGOverworldInteractionComponent::ResolveInteractionSourceActor() const
{
	AActor* OwnerActor = GetOwner();
	if (AController* OwnerController = Cast<AController>(OwnerActor))
	{
		return OwnerController->GetPawn();
	}

	return OwnerActor;
}

void UJGOverworldInteractionComponent::SetFocusedInteractable(AActor* NewFocusedInteractable)
{
	if (FocusedInteractableActor == NewFocusedInteractable)
	{
		return;
	}

	AActor* PreviousInteractable = FocusedInteractableActor;
	FocusedInteractableActor = NewFocusedInteractable;

	UE_LOG(LogJourneyInteraction, Verbose, TEXT("Focused interactable changed from %s to %s"),
		*GetNameSafe(PreviousInteractable),
		*GetNameSafe(FocusedInteractableActor));

	OnFocusedInteractableChanged.Broadcast(PreviousInteractable, FocusedInteractableActor);
}
