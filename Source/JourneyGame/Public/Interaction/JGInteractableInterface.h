#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "JGInteractableInterface.generated.h"

class AActor;

UINTERFACE(BlueprintType)
class JOURNEYGAME_API UJGInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class JOURNEYGAME_API IJGInteractableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	bool CanInteract(AActor* InteractingActor) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	void Interact(AActor* InteractingActor);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	FText GetInteractionPrompt() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	FVector GetInteractionFocusLocation() const;
};
