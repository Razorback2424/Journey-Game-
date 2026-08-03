#pragma once

#include "CoreMinimal.h"
#include "JGOverworldTypes.generated.h"

UENUM(BlueprintType)
enum class EJGTransitionRequestStatus : uint8
{
	None,
	Pending,
	Consumed
};

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGMapTransitionRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overworld")
	FName MapId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overworld")
	FName SpawnTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overworld")
	bool bSeamless = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overworld")
	FString DebugReason;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overworld")
	EJGTransitionRequestStatus Status = EJGTransitionRequestStatus::None;

	bool IsValidRequest() const
	{
		return !MapId.IsNone();
	}
};

