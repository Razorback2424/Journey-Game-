#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/JGInteractableInterface.h"
#include "JGInteractableBase.generated.h"

class UBoxComponent;
class USceneComponent;

UCLASS(Blueprintable)
class JOURNEYGAME_API AJGInteractableBase : public AActor, public IJGInteractableInterface
{
	GENERATED_BODY()

public:
	AJGInteractableBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> InteractionBounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bInteractionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FText PromptText;

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction", meta = (DisplayName = "On Interact"))
	void BP_OnInteract(AActor* InteractingActor);

	virtual bool CanInteract_Implementation(AActor* InteractingActor) const override;
	virtual void Interact_Implementation(AActor* InteractingActor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual FVector GetInteractionFocusLocation_Implementation() const override;
};

