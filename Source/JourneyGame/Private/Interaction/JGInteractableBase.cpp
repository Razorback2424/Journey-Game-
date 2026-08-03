#include "Interaction/JGInteractableBase.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Core/JGLog.h"

AJGInteractableBase::AJGInteractableBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	InteractionBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBounds"));
	InteractionBounds->SetupAttachment(Root);
	InteractionBounds->InitBoxExtent(FVector(16.0f, 16.0f, 16.0f));
	InteractionBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBounds->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionBounds->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBounds->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionBounds->SetGenerateOverlapEvents(false);
}

bool AJGInteractableBase::CanInteract_Implementation(AActor* InteractingActor) const
{
	return bInteractionEnabled && IsValid(InteractingActor);
}

void AJGInteractableBase::Interact_Implementation(AActor* InteractingActor)
{
	if (!IJGInteractableInterface::Execute_CanInteract(this, InteractingActor))
	{
		UE_LOG(LogJourneyInteraction, Verbose, TEXT("Interact ignored for %s"), *GetName());
		return;
	}

	UE_LOG(LogJourneyInteraction, Log, TEXT("%s interacted with %s"), *GetNameSafe(InteractingActor), *GetName());
	BP_OnInteract(InteractingActor);
}

FText AJGInteractableBase::GetInteractionPrompt_Implementation() const
{
	if (!PromptText.IsEmpty())
	{
		return PromptText;
	}
	if (!DisplayName.IsEmpty())
	{
		return FText::Format(NSLOCTEXT("JourneyGame", "InteractWithNamedObject", "Interact: {0}"), DisplayName);
	}
	return NSLOCTEXT("JourneyGame", "InteractDefault", "Interact");
}

FVector AJGInteractableBase::GetInteractionFocusLocation_Implementation() const
{
	return InteractionBounds ? InteractionBounds->GetComponentLocation() : GetActorLocation();
}
