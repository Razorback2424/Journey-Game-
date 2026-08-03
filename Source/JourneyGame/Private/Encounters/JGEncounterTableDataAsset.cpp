#include "Encounters/JGEncounterTableDataAsset.h"

#include "Core/JGLog.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

bool UJGEncounterTableDataAsset::ValidateEncounterTable(FString& OutError) const
{
	return ValidateEncounterTableInternal(&OutError, true);
}

int32 UJGEncounterTableDataAsset::GetTotalEncounterWeight() const
{
	int32 TotalWeight = 0;
	for (const FJGEncounterEntry& Entry : Entries)
	{
		if (Entry.Weight > 0)
		{
			TotalWeight += Entry.Weight;
		}
	}
	return TotalWeight;
}

bool UJGEncounterTableDataAsset::HasValidEntries() const
{
	return ValidateEncounterTableInternal(nullptr, false);
}

bool UJGEncounterTableDataAsset::ValidateEncounterTableInternal(FString* OutError, const bool bLogWarnings) const
{
	auto Fail = [&](const FString& Message) -> bool
	{
		if (OutError)
		{
			*OutError = Message;
		}
		if (bLogWarnings)
		{
			UE_LOG(LogJourneyEncounter, Warning, TEXT("%s"), *Message);
		}
		return false;
	};

	if (!bEnabled)
	{
		return true;
	}

	if (Entries.Num() == 0)
	{
		return Fail(FString::Printf(TEXT("Encounter table '%s' has no entries."), *GetName()));
	}

	int32 ValidWeightSum = 0;
	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		const FJGEncounterEntry& Entry = Entries[Index];
		if (Entry.EncounterId.IsNone())
		{
			return Fail(FString::Printf(TEXT("Encounter table '%s' entry %d has no EncounterId."), *GetName(), Index));
		}
		if (Entry.Weight <= 0)
		{
			return Fail(FString::Printf(TEXT("Encounter table '%s' entry '%s' has non-positive Weight (%d)."), *GetName(), *Entry.EncounterId.ToString(), Entry.Weight));
		}
		if (Entry.MinLevel <= 0 || Entry.MaxLevel <= 0)
		{
			return Fail(FString::Printf(TEXT("Encounter table '%s' entry '%s' has invalid level range (%d-%d)."), *GetName(), *Entry.EncounterId.ToString(), Entry.MinLevel, Entry.MaxLevel));
		}
		if (Entry.MinLevel > Entry.MaxLevel)
		{
			return Fail(FString::Printf(TEXT("Encounter table '%s' entry '%s' has MinLevel > MaxLevel (%d > %d)."), *GetName(), *Entry.EncounterId.ToString(), Entry.MinLevel, Entry.MaxLevel));
		}
		ValidWeightSum += Entry.Weight;
	}

	if (ValidWeightSum <= 0)
	{
		return Fail(FString::Printf(TEXT("Encounter table '%s' has no positive weights."), *GetName()));
	}

	if (OutError)
	{
		OutError->Reset();
	}
	return true;
}

#if WITH_EDITOR
EDataValidationResult UJGEncounterTableDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	FString Error;
	if (!ValidateEncounterTableInternal(&Error, false))
	{
		Context.AddError(FText::FromString(Error));
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}
#endif
