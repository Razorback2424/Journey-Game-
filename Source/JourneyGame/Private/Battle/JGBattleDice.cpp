#include "Battle/JGBattleDice.h"
#include "Internationalization/Regex.h"

bool FJGBattleDice::ParseDiceNotation(const FString& Notation, FJGDiceRollSpec& OutSpec)
{
	const FRegexPattern Pattern(TEXT(R"(^\s*(\d+)d(\d+)([+-]\d+)?\s*$)"));
	FRegexMatcher Matcher(Pattern, Notation);
	if (!Matcher.FindNext())
	{
		return false;
	}

	FJGDiceRollSpec Spec;
	Spec.Count = FCString::Atoi(*Matcher.GetCaptureGroup(1));
	Spec.Sides = FCString::Atoi(*Matcher.GetCaptureGroup(2));
	Spec.Modifier = Matcher.GetCaptureGroup(3).IsEmpty() ? 0 : FCString::Atoi(*Matcher.GetCaptureGroup(3));

	if (!Spec.IsValid())
	{
		return false;
	}

	OutSpec = Spec;
	return true;
}

FJGDiceRollResult FJGBattleDice::Roll(const FJGDiceRollSpec& Spec, FRandomStream& Stream)
{
	FJGDiceRollResult Result;
	if (!Spec.IsValid())
	{
		return Result;
	}

	Result.Rolls.Reserve(Spec.Count);
	int32 Sum = 0;
	for (int32 i = 0; i < Spec.Count; ++i)
	{
		const int32 Value = Stream.RandRange(1, Spec.Sides);
		Result.Rolls.Add(Value);
		Sum += Value;
	}
	Result.Total = FMath::Max(0, Sum + Spec.Modifier);
	return Result;
}

int32 FJGBattleDice::RollD4(FRandomStream& Stream)
{
	return Stream.RandRange(1, 4);
}

int32 FJGBattleDice::RollD20(FRandomStream& Stream)
{
	return Stream.RandRange(1, 20);
}
