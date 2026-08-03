#pragma once

#include "CoreMinimal.h"
#include "JGInteractionTypes.generated.h"

class AActor;

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGInteractionResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bHandled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText PromptOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TObjectPtr<AActor> Target = nullptr;
};

