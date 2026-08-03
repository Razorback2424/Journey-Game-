#pragma once

#include "CoreMinimal.h"
#include "Interaction/JGInteractableBase.h"
#include "JGWobbleTargetBase.generated.h"

UCLASS(Blueprintable)
class JOURNEYGAME_API AJGWobbleTargetBase : public AJGInteractableBase
{
	GENERATED_BODY()

public:
	AJGWobbleTargetBase();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target", meta = (ClampMin = "0.0"))
	float CooldownSeconds = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target")
	bool bSingleUse = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Target")
	bool bTriggered = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Target")
	float LastTriggeredAtSeconds = -1.0f;

	virtual bool CanInteract_Implementation(AActor* InteractingActor) const override;
	virtual void Interact_Implementation(AActor* InteractingActor) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Target", meta = (DisplayName = "On Target Triggered"))
	void BP_OnTargetTriggered(AActor* InteractingActor);
};
