#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Battle/JGBattleDice.h"
#include "Battle/JGBattleSimulation.h"
#include "Battle/JGBattleSubsystem.h"
#include "Battle/Data/JGBattleEncounterDefinitionDataAsset.h"
#include "Battle/Data/JGBattleUnitArchetypeDataAsset.h"
#include "Encounters/JGEncounterTableDataAsset.h"
#include "Overworld/JGOverworldGameModeBase.h"
#include "Overworld/JGOverworldSubsystem.h"
#include "Engine/GameInstance.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJGBattleDiceParsingTest,
	"JourneyGame.Battle.DiceParsing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FJGBattleDiceParsingTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	FJGDiceRollSpec Spec;
	TestTrue(TEXT("Parses 1d6+2"), FJGBattleDice::ParseDiceNotation(TEXT("1d6+2"), Spec));
	TestEqual(TEXT("Count"), Spec.Count, 1);
	TestEqual(TEXT("Sides"), Spec.Sides, 6);
	TestEqual(TEXT("Modifier"), Spec.Modifier, 2);

	TestTrue(TEXT("Parses 2d8"), FJGBattleDice::ParseDiceNotation(TEXT("2d8"), Spec));
	TestEqual(TEXT("Count2"), Spec.Count, 2);
	TestEqual(TEXT("Sides2"), Spec.Sides, 8);
	TestEqual(TEXT("Modifier2"), Spec.Modifier, 0);

	TestTrue(TEXT("Parses 1d4-1"), FJGBattleDice::ParseDiceNotation(TEXT("1d4-1"), Spec));
	TestEqual(TEXT("Modifier3"), Spec.Modifier, -1);

	TestFalse(TEXT("Rejects invalid"), FJGBattleDice::ParseDiceNotation(TEXT("abc"), Spec));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJGBattleDiceDeterminismTest,
	"JourneyGame.Battle.DiceDeterminism",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FJGBattleDiceDeterminismTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	FJGDiceRollSpec Spec;
	TestTrue(TEXT("Parse dice"), FJGBattleDice::ParseDiceNotation(TEXT("2d6+1"), Spec));

	FRandomStream A(42);
	FRandomStream B(42);
	const FJGDiceRollResult RollA = FJGBattleDice::Roll(Spec, A);
	const FJGDiceRollResult RollB = FJGBattleDice::Roll(Spec, B);
	TestEqual(TEXT("Same total with same seed"), RollA.Total, RollB.Total);
	TestEqual(TEXT("Same roll count"), RollA.Rolls.Num(), RollB.Rolls.Num());
	if (RollA.Rolls.Num() == RollB.Rolls.Num())
	{
		for (int32 i = 0; i < RollA.Rolls.Num(); ++i)
		{
			TestEqual(FString::Printf(TEXT("Roll %d"), i), RollA.Rolls[i], RollB.Rolls[i]);
		}
	}
	return true;
}

static UJGBattleUnitArchetypeDataAsset* JGMakeArchetype(const TCHAR* Id, EJGBattleUnitRole Role, int32 MoveRange, int32 AttackRange)
{
	UJGBattleUnitArchetypeDataAsset* A = NewObject<UJGBattleUnitArchetypeDataAsset>();
	A->ArchetypeId = FName(Id);
	A->Role = Role;
	A->MaxHp = 10;
	A->AC = 12;
	A->AttackBonus = 3;
	A->InitiativeBonus = 1;
	A->DamageDice = TEXT("1d4+1");
	A->MoveRange = MoveRange;
	A->AttackRange = AttackRange;
	return A;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJGBattleEncounterDefinitionValidationTest,
	"JourneyGame.Battle.EncounterDefinitionValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FJGBattleEncounterDefinitionValidationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	UJGBattleEncounterDefinitionDataAsset* Def = NewObject<UJGBattleEncounterDefinitionDataAsset>();
	Def->BattleId = TEXT("TestBattle");
	Def->GridWidth = 6;
	Def->GridHeight = 6;

	UJGBattleUnitArchetypeDataAsset* PlayerArch = JGMakeArchetype(TEXT("Hero"), EJGBattleUnitRole::Guardian, 3, 1);
	UJGBattleUnitArchetypeDataAsset* EnemyArch = JGMakeArchetype(TEXT("Goblin"), EJGBattleUnitRole::Raider, 4, 1);

	Def->PlayerSlots = {
		{ PlayerArch, FJGBattleGridCoord(0,0), INDEX_NONE, FName(TEXT("p-1")) },
		{ PlayerArch, FJGBattleGridCoord(0,1), INDEX_NONE, FName(TEXT("p-2")) },
		{ PlayerArch, FJGBattleGridCoord(0,2), INDEX_NONE, FName(TEXT("p-3")) }
	};
	Def->EnemySlots = {
		{ EnemyArch, FJGBattleGridCoord(4,4), INDEX_NONE, FName(TEXT("e-1")) }
	};

	FString Error;
	TestTrue(TEXT("Valid definition passes"), Def->ValidateDefinition(Error));
	TestTrue(TEXT("No error text"), Error.IsEmpty());

	Def->EnemySlots[0].SpawnCoord = FJGBattleGridCoord(0,0);
	TestFalse(TEXT("Spawn overlap fails"), Def->ValidateDefinition(Error));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJGBattleBasicFlowTest,
	"JourneyGame.Battle.BasicFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FJGBattleBasicFlowTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	UJGBattleUnitArchetypeDataAsset* Guardian = JGMakeArchetype(TEXT("Guardian"), EJGBattleUnitRole::Guardian, 3, 1);
	Guardian->BonusAbilityType = EJGBattleBonusAbilityType::Brace;
	UJGBattleUnitArchetypeDataAsset* Striker = JGMakeArchetype(TEXT("Striker"), EJGBattleUnitRole::Striker, 3, 1);
	Striker->BonusAbilityType = EJGBattleBonusAbilityType::Focus;
	UJGBattleUnitArchetypeDataAsset* Scout = JGMakeArchetype(TEXT("Scout"), EJGBattleUnitRole::Scout, 4, 2);
	Scout->BonusAbilityType = EJGBattleBonusAbilityType::Quickstep;
	UJGBattleUnitArchetypeDataAsset* Enemy = JGMakeArchetype(TEXT("Enemy"), EJGBattleUnitRole::Brute, 2, 1);

	UJGBattleEncounterDefinitionDataAsset* Def = NewObject<UJGBattleEncounterDefinitionDataAsset>();
	Def->BattleId = TEXT("FlowTest");
	Def->GridWidth = 8;
	Def->GridHeight = 8;
	Def->PlayerSlots = {
		{ Guardian, FJGBattleGridCoord(1,1), INDEX_NONE, FName(TEXT("p-1")) },
		{ Striker, FJGBattleGridCoord(1,3), INDEX_NONE, FName(TEXT("p-2")) },
		{ Scout, FJGBattleGridCoord(1,5), INDEX_NONE, FName(TEXT("p-3")) }
	};
	Def->EnemySlots = {
		{ Enemy, FJGBattleGridCoord(4,1), INDEX_NONE, FName(TEXT("e-1")) }
	};

	FJGBattleState State;
	FJGBattleCommandResult Result;
	FRandomStream Stream(7);
	TestTrue(TEXT("Init succeeds"), FJGBattleSimulation::InitializeFromEncounter(Def, Stream, State, Result));
	TestTrue(TEXT("Units created"), State.Units.Num() == 4);
	TestTrue(TEXT("Turn order created"), State.TurnOrder.Num() == 4);

	// Basic sanity: current turn starts and command execution returns a result.
	FJGBattleCommand SelectCmd;
	SelectCmd.Type = EJGBattleCommandType::SelectUnit;
	SelectCmd.UnitHandle = State.TurnOrder[State.CurrentTurnIndex];
	Result = FJGBattleSimulation::ExecuteCommand(State, SelectCmd, Stream);
	TestTrue(TEXT("Select command returns result"), Result.bSuccess || Result.FailureReason != EJGBattleFailureReason::None);

	return true;
}

static UJGBattleEncounterDefinitionDataAsset* JGMakeSimpleBattleDefinition()
{
	UJGBattleUnitArchetypeDataAsset* Player = JGMakeArchetype(TEXT("Hero"), EJGBattleUnitRole::Guardian, 3, 1);
	Player->InitiativeBonus = 5;
	UJGBattleUnitArchetypeDataAsset* Enemy = JGMakeArchetype(TEXT("Enemy"), EJGBattleUnitRole::Brute, 2, 1);

	UJGBattleEncounterDefinitionDataAsset* Def = NewObject<UJGBattleEncounterDefinitionDataAsset>();
	Def->BattleId = TEXT("SubsystemFlowTest");
	Def->GridWidth = 6;
	Def->GridHeight = 6;
	Def->PlayerSlots = {
		{ Player, FJGBattleGridCoord(1,1), INDEX_NONE, FName(TEXT("p-1")) }
	};
	Def->EnemySlots = {
		{ Enemy, FJGBattleGridCoord(4,1), INDEX_NONE, FName(TEXT("e-1")) }
	};
	return Def;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJGBattleSubsystemLifecycleTest,
	"JourneyGame.Battle.Subsystem.Lifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FJGBattleSubsystemLifecycleTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UJGBattleSubsystem* BattleSubsystem = NewObject<UJGBattleSubsystem>(GameInstance);
	TestNotNull(TEXT("Battle subsystem created"), BattleSubsystem);
	if (!BattleSubsystem)
	{
		return false;
	}

	FJGBattleStartRequest InvalidRequest;
	FJGBattleCommandResult Result;
	TestFalse(TEXT("Invalid request is rejected"), BattleSubsystem->RequestBattleStart(InvalidRequest, Result));
	TestFalse(TEXT("Rejected request does not create pending battle"), BattleSubsystem->HasPendingBattleStart());

	FJGBattleStartRequest Request;
	Request.EncounterDefinition = JGMakeSimpleBattleDefinition();
	Request.Seed = 17;
	Request.SourceEncounterId = TEXT("TutorialFight");
	Request.ReturnMapId = TEXT("Lvl_Overworld_Test");
	Request.ReturnSpawnTag = TEXT("EncounterReturn");

	TestTrue(TEXT("Valid battle start request is accepted"), BattleSubsystem->RequestBattleStart(Request, Result));
	TestTrue(TEXT("Pending battle start is tracked"), BattleSubsystem->HasPendingBattleStart());
	TestFalse(TEXT("Battle is not active until initialized"), BattleSubsystem->HasActiveBattle());

	TestTrue(TEXT("Pending battle initializes active battle"), BattleSubsystem->InitializeActiveBattle(Result));
	TestFalse(TEXT("Pending battle is consumed after initialization"), BattleSubsystem->HasPendingBattleStart());
	TestTrue(TEXT("Active battle exists after initialization"), BattleSubsystem->HasActiveBattle());
	TestEqual(TEXT("Active battle keeps source encounter id"), BattleSubsystem->GetActiveBattleStartRequest().SourceEncounterId, FName(TEXT("TutorialFight")));
	TestTrue(TEXT("Initialized battle has units"), BattleSubsystem->GetActiveBattleState().Units.Num() == 2);

	FJGBattleCommand SelectCommand;
	SelectCommand.Type = EJGBattleCommandType::SelectUnit;
	SelectCommand.UnitHandle = BattleSubsystem->GetActiveBattleState().TurnOrder[BattleSubsystem->GetActiveBattleState().CurrentTurnIndex];
	int32 StreamAdvance = INDEX_NONE;
	TestTrue(TEXT("Active battle accepts command execution"), BattleSubsystem->ExecuteActiveBattleCommand(SelectCommand, Result, StreamAdvance));
	TestTrue(TEXT("Stream advance is reported"), StreamAdvance >= 0);

	FJGBattleStartRequest EndedRequest;
	TestTrue(TEXT("Ending active battle succeeds"), BattleSubsystem->EndActiveBattle(EJGBattleTeam::Player, EndedRequest));
	TestFalse(TEXT("No active battle remains after ending"), BattleSubsystem->HasActiveBattle());
	TestEqual(TEXT("Ended battle preserves return map"), EndedRequest.ReturnMapId, FName(TEXT("Lvl_Overworld_Test")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJGEncounterToBattleFlowTest,
	"JourneyGame.Battle.EncounterToBattleFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FJGEncounterToBattleFlowTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UJGOverworldSubsystem* OverworldSubsystem = NewObject<UJGOverworldSubsystem>(GameInstance);
	UJGBattleSubsystem* BattleSubsystem = NewObject<UJGBattleSubsystem>(GameInstance);
	AJGOverworldGameModeBase* GameMode = NewObject<AJGOverworldGameModeBase>();

	TestNotNull(TEXT("Overworld subsystem created"), OverworldSubsystem);
	TestNotNull(TEXT("Battle subsystem created"), BattleSubsystem);
	TestNotNull(TEXT("Game mode created"), GameMode);
	if (!OverworldSubsystem || !BattleSubsystem || !GameMode)
	{
		return false;
	}

	UJGEncounterTableDataAsset* Table = NewObject<UJGEncounterTableDataAsset>();
	FJGEncounterEntry Entry;
	Entry.EncounterId = TEXT("TutorialFight");
	Entry.Weight = 1;
	Entry.MinLevel = 1;
	Entry.MaxLevel = 1;
	Entry.BattlePayload = JGMakeSimpleBattleDefinition();
	Table->Entries = { Entry };

	FJGEncounterRequest Request;
	Request.EncounterTable = Table;
	Request.SourceActor = NewObject<AActor>();
	Request.TriggeringActor = NewObject<AActor>();
	TestTrue(TEXT("Pending encounter is accepted"), OverworldSubsystem->RequestEncounter(Request));

	FJGBattleCommandResult Result;
	TestTrue(TEXT("Pending encounter is converted to battle start"), GameMode->TryProcessPendingEncounterForBattleWithSubsystems(OverworldSubsystem, BattleSubsystem, Result));
	TestFalse(TEXT("Pending encounter is consumed"), OverworldSubsystem->HasPendingEncounter());
	TestTrue(TEXT("Battle start request is pending"), BattleSubsystem->HasPendingBattleStart());

	TestTrue(TEXT("Pending battle initializes"), BattleSubsystem->InitializeActiveBattle(Result));
	TestTrue(TEXT("Active battle exists"), BattleSubsystem->HasActiveBattle());

	UJGEncounterTableDataAsset* MissingPayloadTable = NewObject<UJGEncounterTableDataAsset>();
	FJGEncounterEntry MissingPayloadEntry;
	MissingPayloadEntry.EncounterId = TEXT("MissingPayload");
	MissingPayloadEntry.Weight = 1;
	MissingPayloadEntry.MinLevel = 1;
	MissingPayloadEntry.MaxLevel = 1;
	MissingPayloadTable->Entries = { MissingPayloadEntry };
	Request.EncounterTable = MissingPayloadTable;
	TestTrue(TEXT("Pending encounter with missing payload is accepted"), OverworldSubsystem->RequestEncounter(Request));

	BattleSubsystem->ClearPendingBattleStart();
	TestFalse(TEXT("Missing payload does not start battle"), GameMode->TryProcessPendingEncounterForBattleWithSubsystems(OverworldSubsystem, BattleSubsystem, Result));
	TestFalse(TEXT("Missing payload leaves no pending encounter"), OverworldSubsystem->HasPendingEncounter());
	TestFalse(TEXT("Missing payload leaves no pending battle"), BattleSubsystem->HasPendingBattleStart());

	return true;
}

#endif
