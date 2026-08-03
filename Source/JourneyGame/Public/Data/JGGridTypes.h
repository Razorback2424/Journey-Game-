#pragma once

#include "CoreMinimal.h"
#include "JGGridTypes.generated.h"

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGGridCoord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 X = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 Y = 0;

	FJGGridCoord() = default;
	FJGGridCoord(const int32 InX, const int32 InY)
		: X(InX), Y(InY)
	{
	}

	bool operator==(const FJGGridCoord& Other) const
	{
		return X == Other.X && Y == Other.Y;
	}
};

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGGridSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ClampMin = "1.0"))
	float TileSizeUU = 32.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	bool bSnapOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ClampMin = "0.01"))
	float StepDuration = 0.14f;
};

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGOverworldSpawnPointRef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overworld")
	FName MapId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overworld")
	FName SpawnTag = NAME_None;
};

