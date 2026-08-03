#pragma once

#include "CoreMinimal.h"
#include "Interaction/JGInteractableBase.h"
#include "JGDoorInteractableBase.generated.h"

UCLASS(Blueprintable)
class JOURNEYGAME_API AJGDoorInteractableBase : public AJGInteractableBase
{
	GENERATED_BODY()

public:
	AJGDoorInteractableBase();

	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
	bool bStartsOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
	bool bLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door")
	bool bToggleOnInteract = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Door")
	bool bIsOpen = false;

	UFUNCTION(BlueprintCallable, Category = "Door")
	void SetDoorLocked(bool bInLocked);

	virtual bool CanInteract_Implementation(AActor* InteractingActor) const override;
	virtual void Interact_Implementation(AActor* InteractingActor) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Door", meta = (DisplayName = "On Door Opened"))
	void BP_OnDoorOpened(AActor* InteractingActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Door", meta = (DisplayName = "On Door Closed"))
	void BP_OnDoorClosed(AActor* InteractingActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Door", meta = (DisplayName = "On Door Locked Interaction"))
	void BP_OnLockedInteraction(AActor* InteractingActor);
};
