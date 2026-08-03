#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Encounters/JGEncounterBlueprintLibrary.h"
#include "Encounters/JGEncounterTableDataAsset.h"
#include "Overworld/JGGridBlueprintLibrary.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJGGridSnapWorldLocationTest,
	"JourneyGame.Grid.SnapWorldLocationToGrid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FJGGridSnapWorldLocationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	const FVector Input(47.0f, -17.0f, 123.0f);
	const FVector Snapped = UJGGridBlueprintLibrary::SnapWorldLocationToGrid(Input, 32.0f, true, 0.0f);

	TestEqual(TEXT("X snaps to nearest 32"), Snapped.X, 32.0, 0.001);
	TestEqual(TEXT("Y snaps to nearest 32"), Snapped.Y, -32.0, 0.001);
	TestEqual(TEXT("Z preserved"), Snapped.Z, 123.0, 0.001);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJGGridCoordRoundTripTest,
	"JourneyGame.Grid.GridCoordRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FJGGridCoordRoundTripTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	const FJGGridCoord Coord = UJGGridBlueprintLibrary::GridCoordFromWorld(FVector(64.0f, -96.0f, 0.0f), 32.0f);
	TestEqual(TEXT("Grid X"), Coord.X, 2);
	TestEqual(TEXT("Grid Y"), Coord.Y, -3);

	const FVector World = UJGGridBlueprintLibrary::WorldFromGridCoord(Coord, 32.0f, 5.0f);
	TestEqual(TEXT("Round-trip world X"), World.X, 64.0, 0.001);
	TestEqual(TEXT("Round-trip world Y"), World.Y, -96.0, 0.001);
	TestEqual(TEXT("Round-trip world Z"), World.Z, 5.0, 0.001);
	TestTrue(TEXT("Location is on grid"), UJGGridBlueprintLibrary::IsWorldLocationOnGrid(World, 32.0f, 0.01f));
	TestEqual(TEXT("Grid step world delta"), UJGGridBlueprintLibrary::GridStepToWorldDelta(FVector2D(0.2f, 1.0f), 32.0f), FVector(0.0f, 32.0f, 0.0f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJGEncounterTableValidationTest,
	"JourneyGame.Encounter.TableValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FJGEncounterTableValidationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	UJGEncounterTableDataAsset* Table = NewObject<UJGEncounterTableDataAsset>();
	TestNotNull(TEXT("Encounter table object created"), Table);
	if (!Table)
	{
		return false;
	}

	FString Error;
	TestFalse(TEXT("Empty table is invalid"), Table->ValidateEncounterTable(Error));
	TestTrue(TEXT("Empty table returns error message"), !Error.IsEmpty());

	FJGEncounterEntry InvalidEntry;
	InvalidEntry.EncounterId = TEXT("Slime");
	InvalidEntry.Weight = 0;
	InvalidEntry.MinLevel = 1;
	InvalidEntry.MaxLevel = 2;
	Table->Entries = { InvalidEntry };

	Error.Reset();
	TestFalse(TEXT("Non-positive weight is invalid"), Table->ValidateEncounterTable(Error));
	TestTrue(TEXT("Weight error mentions something"), !Error.IsEmpty());

	FJGEncounterEntry ValidEntry;
	ValidEntry.EncounterId = TEXT("Bat");
	ValidEntry.Weight = 10;
	ValidEntry.MinLevel = 2;
	ValidEntry.MaxLevel = 4;
	Table->Entries = { ValidEntry };

	Error.Reset();
	TestTrue(TEXT("Valid table passes validation"), Table->ValidateEncounterTable(Error));
	TestTrue(TEXT("No error on valid table"), Error.IsEmpty());
	TestEqual(TEXT("Total weight sums valid entries"), Table->GetTotalEncounterWeight(), 10);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJGEncounterWeightedPickerTest,
	"JourneyGame.Encounter.WeightedPicker",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FJGEncounterWeightedPickerTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	UJGEncounterTableDataAsset* Table = NewObject<UJGEncounterTableDataAsset>();
	TestNotNull(TEXT("Encounter table object created"), Table);
	if (!Table)
	{
		return false;
	}

	FJGEncounterEntry Slime;
	Slime.EncounterId = TEXT("Slime");
	Slime.Weight = 2;
	Slime.MinLevel = 1;
	Slime.MaxLevel = 2;

	FJGEncounterEntry Bat;
	Bat.EncounterId = TEXT("Bat");
	Bat.Weight = 3;
	Bat.MinLevel = 1;
	Bat.MaxLevel = 3;

	Table->Entries = { Slime, Bat };

	TestEqual(TEXT("Total weight from picker library"), UJGEncounterBlueprintLibrary::GetEncounterTableTotalWeight(Table), 5);

	FJGEncounterEntry Picked;
	TestTrue(TEXT("Roll 0 picks first entry"), UJGEncounterBlueprintLibrary::TryPickEncounterByRoll(Table, 0, Picked));
	TestEqual(TEXT("Roll 0 result"), Picked.EncounterId, FName(TEXT("Slime")));

	TestTrue(TEXT("Roll 1 picks first entry"), UJGEncounterBlueprintLibrary::TryPickEncounterByRoll(Table, 1, Picked));
	TestEqual(TEXT("Roll 1 result"), Picked.EncounterId, FName(TEXT("Slime")));

	TestTrue(TEXT("Roll 2 picks second entry"), UJGEncounterBlueprintLibrary::TryPickEncounterByRoll(Table, 2, Picked));
	TestEqual(TEXT("Roll 2 result"), Picked.EncounterId, FName(TEXT("Bat")));

	TestTrue(TEXT("Roll 4 picks second entry"), UJGEncounterBlueprintLibrary::TryPickEncounterByRoll(Table, 4, Picked));
	TestEqual(TEXT("Roll 4 result"), Picked.EncounterId, FName(TEXT("Bat")));

	FRandomStream Stream(12345);
	int32 Roll = INDEX_NONE;
	TestTrue(TEXT("Stream-based pick succeeds"), UJGEncounterBlueprintLibrary::TryPickEncounterFromStream(Table, Stream, Picked, Roll));
	TestTrue(TEXT("Stream roll in range"), Roll >= 0 && Roll < 5);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
