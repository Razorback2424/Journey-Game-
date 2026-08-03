#include "Battle/Data/JGBattleUnitArchetypeDataAsset.h"

#include "Battle/JGBattleDice.h"

bool UJGBattleUnitArchetypeDataAsset::ValidateArchetype(FString& OutError) const
{
	if (ArchetypeId.IsNone())
	{
		OutError = TEXT("ArchetypeId is required.");
		return false;
	}
	if (MaxHp <= 0 || AC <= 0 || MoveRange <= 0 || AttackRange <= 0)
	{
		OutError = TEXT("MaxHp, AC, MoveRange, and AttackRange must be > 0.");
		return false;
	}
	FJGDiceRollSpec DiceSpec;
	if (!FJGBattleDice::ParseDiceNotation(DamageDice, DiceSpec))
	{
		OutError = FString::Printf(TEXT("Invalid DamageDice notation: %s"), *DamageDice);
		return false;
	}
	OutError.Reset();
	return true;
}

#if WITH_EDITOR
EDataValidationResult UJGBattleUnitArchetypeDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	FString Error;
	if (!ValidateArchetype(Error))
	{
		Context.AddError(FText::FromString(Error));
		return EDataValidationResult::Invalid;
	}
	return EDataValidationResult::Valid;
}
#endif

