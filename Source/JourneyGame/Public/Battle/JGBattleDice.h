#pragma once

#include "CoreMinimal.h"
#include "JGBattleDice.generated.h"

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGDiceRollSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 Count = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 Sides = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 Modifier = 0;

	bool IsValid() const { return Count > 0 && Sides > 0; }
};

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGDiceRollResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	TArray<int32> Rolls;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 Total = 0;
};

struct JOURNEYGAME_API FJGBattleDice
{
	static bool ParseDiceNotation(const FString& Notation, FJGDiceRollSpec& OutSpec);
	static FJGDiceRollResult Roll(const FJGDiceRollSpec& Spec, FRandomStream& Stream);
	static int32 RollD4(FRandomStream& Stream);
	static int32 RollD20(FRandomStream& Stream);
};

