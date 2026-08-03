#include "Interaction/JGDoorInteractableBase.h"

#include "Core/JGLog.h"

AJGDoorInteractableBase::AJGDoorInteractableBase()
{
	bIsOpen = bStartsOpen;
}

void AJGDoorInteractableBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	bIsOpen = bStartsOpen;
}

void AJGDoorInteractableBase::SetDoorLocked(const bool bInLocked)
{
	bLocked = bInLocked;
}

bool AJGDoorInteractableBase::CanInteract_Implementation(AActor* InteractingActor) const
{
	return Super::CanInteract_Implementation(InteractingActor);
}

void AJGDoorInteractableBase::Interact_Implementation(AActor* InteractingActor)
{
	if (!Super::CanInteract_Implementation(InteractingActor))
	{
		return;
	}

	if (bLocked)
	{
		UE_LOG(LogJourneyInteraction, Verbose, TEXT("Locked door interaction blocked for %s"), *GetName());
		BP_OnLockedInteraction(InteractingActor);
		return;
	}

	const bool bShouldOpen = bToggleOnInteract ? !bIsOpen : true;
	bIsOpen = bShouldOpen;
	BP_OnInteract(InteractingActor);

	if (bIsOpen)
	{
		BP_OnDoorOpened(InteractingActor);
	}
	else
	{
		BP_OnDoorClosed(InteractingActor);
	}
}
