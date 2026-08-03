#include "Interaction/JGWobbleTargetBase.h"

#include "Core/JGLog.h"
#include "Engine/World.h"

AJGWobbleTargetBase::AJGWobbleTargetBase()
{
}

bool AJGWobbleTargetBase::CanInteract_Implementation(AActor* InteractingActor) const
{
	if (!Super::CanInteract_Implementation(InteractingActor))
	{
		return false;
	}

	if (bSingleUse && bTriggered)
	{
		return false;
	}

	if (const UWorld* World = GetWorld())
	{
		if (LastTriggeredAtSeconds >= 0.0f && (World->GetTimeSeconds() - LastTriggeredAtSeconds) < CooldownSeconds)
		{
			return false;
		}
	}

	return true;
}

void AJGWobbleTargetBase::Interact_Implementation(AActor* InteractingActor)
{
	if (!CanInteract_Implementation(InteractingActor))
	{
		UE_LOG(LogJourneyInteraction, Verbose, TEXT("Wobble target %s ignored interaction"), *GetName());
		return;
	}

	bTriggered = true;
	if (UWorld* World = GetWorld())
	{
		LastTriggeredAtSeconds = World->GetTimeSeconds();
	}

	BP_OnInteract(InteractingActor);
	BP_OnTargetTriggered(InteractingActor);
}
