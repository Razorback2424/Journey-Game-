#include "Battle/Data/JGBattleEncounterDefinitionDataAsset.h"

#include "Battle/Data/JGBattleUnitArchetypeDataAsset.h"

static bool JGIsCoordInside(const FJGBattleGridCoord& Coord, int32 Width, int32 Height)
{
	return Coord.X >= 0 && Coord.X < Width && Coord.Y >= 0 && Coord.Y < Height;
}

bool UJGBattleEncounterDefinitionDataAsset::ValidateDefinition(FString& OutError) const
{
	if (BattleId.IsNone())
	{
		OutError = TEXT("BattleId is required.");
		return false;
	}
	if (GridWidth < 3 || GridHeight < 3)
	{
		OutError = TEXT("GridWidth and GridHeight must be at least 3.");
		return false;
	}
	if (PlayerSlots.Num() != 3)
	{
		OutError = FString::Printf(TEXT("PlayerSlots must contain exactly 3 units (found %d)."), PlayerSlots.Num());
		return false;
	}
	if (EnemySlots.Num() < 1 || EnemySlots.Num() > 5)
	{
		OutError = FString::Printf(TEXT("EnemySlots must contain 1 to 5 units (found %d)."), EnemySlots.Num());
		return false;
	}

	TSet<FJGBattleGridCoord> BlockerSet;
	for (const FJGBattleGridCoord& Blocker : Blockers)
	{
		if (!JGIsCoordInside(Blocker, GridWidth, GridHeight))
		{
			OutError = FString::Printf(TEXT("Blocker out of bounds at (%d,%d)."), Blocker.X, Blocker.Y);
			return false;
		}
		BlockerSet.Add(Blocker);
	}

	TSet<FJGBattleGridCoord> Occupied;
	auto ValidateSlots = [&](const TArray<FJGBattleEncounterSlot>& Slots, const TCHAR* Label) -> bool
	{
		for (int32 i = 0; i < Slots.Num(); ++i)
		{
			const FJGBattleEncounterSlot& Slot = Slots[i];
			if (!Slot.Archetype)
			{
				OutError = FString::Printf(TEXT("%s slot %d missing Archetype."), Label, i);
				return false;
			}
			if (!JGIsCoordInside(Slot.SpawnCoord, GridWidth, GridHeight))
			{
				OutError = FString::Printf(TEXT("%s slot %d spawn out of bounds (%d,%d)."), Label, i, Slot.SpawnCoord.X, Slot.SpawnCoord.Y);
				return false;
			}
			if (BlockerSet.Contains(Slot.SpawnCoord))
			{
				OutError = FString::Printf(TEXT("%s slot %d spawn overlaps blocker (%d,%d)."), Label, i, Slot.SpawnCoord.X, Slot.SpawnCoord.Y);
				return false;
			}
			if (Occupied.Contains(Slot.SpawnCoord))
			{
				OutError = FString::Printf(TEXT("%s slot %d spawn overlaps another unit (%d,%d)."), Label, i, Slot.SpawnCoord.X, Slot.SpawnCoord.Y);
				return false;
			}
			Occupied.Add(Slot.SpawnCoord);
		}
		return true;
	};

	if (!ValidateSlots(PlayerSlots, TEXT("Player")))
	{
		return false;
	}
	if (!ValidateSlots(EnemySlots, TEXT("Enemy")))
	{
		return false;
	}

	OutError.Reset();
	return true;
}

#if WITH_EDITOR
EDataValidationResult UJGBattleEncounterDefinitionDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	FString Error;
	if (!ValidateDefinition(Error))
	{
		Context.AddError(FText::FromString(Error));
		return EDataValidationResult::Invalid;
	}
	return EDataValidationResult::Valid;
}
#endif

