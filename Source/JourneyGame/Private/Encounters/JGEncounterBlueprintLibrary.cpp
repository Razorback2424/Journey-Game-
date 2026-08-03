#include "Encounters/JGEncounterBlueprintLibrary.h"

#include "Encounters/JGEncounterTableDataAsset.h"

namespace JGEncounterPickerInternal
{
	static bool IsEntryPickable(const FJGEncounterEntry& Entry)
	{
		return !Entry.EncounterId.IsNone() && Entry.Weight > 0 && Entry.MinLevel > 0 && Entry.MaxLevel >= Entry.MinLevel;
	}

	static int32 ComputeTotalWeight(const UJGEncounterTableDataAsset* EncounterTable)
	{
		if (!EncounterTable)
		{
			return 0;
		}

		int32 TotalWeight = 0;
		for (const FJGEncounterEntry& Entry : EncounterTable->Entries)
		{
			if (IsEntryPickable(Entry))
			{
				TotalWeight += Entry.Weight;
			}
		}
		return TotalWeight;
	}

	static bool PickByRoll(const UJGEncounterTableDataAsset* EncounterTable, const int32 RollValue, FJGEncounterEntry& OutEntry)
	{
		if (!EncounterTable)
		{
			return false;
		}

		const int32 TotalWeight = ComputeTotalWeight(EncounterTable);
		if (TotalWeight <= 0)
		{
			return false;
		}

		const int32 ClampedRoll = FMath::Clamp(RollValue, 0, TotalWeight - 1);
		int32 RunningWeight = 0;

		for (const FJGEncounterEntry& Entry : EncounterTable->Entries)
		{
			if (!IsEntryPickable(Entry))
			{
				continue;
			}

			RunningWeight += Entry.Weight;
			if (ClampedRoll < RunningWeight)
			{
				OutEntry = Entry;
				return true;
			}
		}

		return false;
	}
}

int32 UJGEncounterBlueprintLibrary::GetEncounterTableTotalWeight(const UJGEncounterTableDataAsset* EncounterTable)
{
	return JGEncounterPickerInternal::ComputeTotalWeight(EncounterTable);
}

bool UJGEncounterBlueprintLibrary::TryPickEncounterByRoll(
	const UJGEncounterTableDataAsset* EncounterTable,
	const int32 RollValue,
	FJGEncounterEntry& OutEntry)
{
	return JGEncounterPickerInternal::PickByRoll(EncounterTable, RollValue, OutEntry);
}

bool UJGEncounterBlueprintLibrary::TryPickEncounterRandom(
	const UJGEncounterTableDataAsset* EncounterTable,
	FJGEncounterEntry& OutEntry,
	int32& OutRollValue)
{
	const int32 TotalWeight = JGEncounterPickerInternal::ComputeTotalWeight(EncounterTable);
	if (TotalWeight <= 0)
	{
		OutRollValue = INDEX_NONE;
		return false;
	}

	OutRollValue = FMath::RandRange(0, TotalWeight - 1);
	return JGEncounterPickerInternal::PickByRoll(EncounterTable, OutRollValue, OutEntry);
}

bool UJGEncounterBlueprintLibrary::TryPickEncounterFromStream(
	const UJGEncounterTableDataAsset* EncounterTable,
	FRandomStream& Stream,
	FJGEncounterEntry& OutEntry,
	int32& OutRollValue)
{
	const int32 TotalWeight = JGEncounterPickerInternal::ComputeTotalWeight(EncounterTable);
	if (TotalWeight <= 0)
	{
		OutRollValue = INDEX_NONE;
		return false;
	}

	OutRollValue = Stream.RandRange(0, TotalWeight - 1);
	return JGEncounterPickerInternal::PickByRoll(EncounterTable, OutRollValue, OutEntry);
}

