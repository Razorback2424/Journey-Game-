#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Encounters/JGEncounterTableDataAsset.h"
#include "Encounters/JGEncounterZoneBase.h"
#include "GameFramework/Actor.h"
#include "Interaction/JGInteractableBase.h"
#include "Overworld/JGMapTransitionTriggerBase.h"
#include "Overworld/JGOverworldCharacterBase.h"
#include "Overworld/JGOverworldGameModeBase.h"
#include "Overworld/JGOverworldInteractionComponent.h"
#include "Overworld/JGOverworldPlayerControllerBase.h"
#include "Overworld/JGOverworldSubsystem.h"
#include "Engine/GameInstance.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJGOverworldInteractionFocusAndPromptTest,
	"JourneyGame.Overworld.Interaction.FocusAndPrompt",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FJGOverworldInteractionFocusAndPromptTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	AActor* SourceActor = NewObject<AActor>();
	AJGInteractableBase* Interactable = NewObject<AJGInteractableBase>();
	UJGOverworldInteractionComponent* InteractionComponent = NewObject<UJGOverworldInteractionComponent>(SourceActor);

	TestNotNull(TEXT("Source actor created"), SourceActor);
	TestNotNull(TEXT("Interactable created"), Interactable);
	TestNotNull(TEXT("Interaction component created"), InteractionComponent);
	if (!SourceActor || !Interactable || !InteractionComponent)
	{
		return false;
	}

	InteractionComponent->DefaultPromptText = FText::FromString(TEXT("Fallback prompt"));
	Interactable->PromptText = FText::FromString(TEXT("Talk"));

	TestTrue(TEXT("Focused interactable can be updated from a valid actor"), InteractionComponent->UpdateFocusedInteractable(Interactable));
	TestTrue(TEXT("Component tracks focused interactable"), InteractionComponent->HasFocusedInteractable());
	TestEqual(TEXT("Focused interactable matches"), InteractionComponent->GetFocusedInteractable(), static_cast<AActor*>(Interactable));
	TestEqual(TEXT("Focused prompt uses interactable prompt"), InteractionComponent->GetFocusedPromptText().ToString(), FString(TEXT("Talk")));

	Interactable->PromptText = FText::GetEmpty();
	TestEqual(TEXT("Fallback prompt is used when interactable prompt is empty"), InteractionComponent->GetFocusedPromptText().ToString(), FString(TEXT("Fallback prompt")));

	InteractionComponent->bInteractionEnabled = false;
	TestFalse(TEXT("Disabled component clears focus"), InteractionComponent->UpdateFocusedInteractable(Interactable));
	TestFalse(TEXT("No focused interactable remains when disabled"), InteractionComponent->HasFocusedInteractable());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJGOverworldInteractionTryInteractTest,
	"JourneyGame.Overworld.Interaction.TryInteract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FJGOverworldInteractionTryInteractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	AActor* SourceActor = NewObject<AActor>();
	AJGInteractableBase* Interactable = NewObject<AJGInteractableBase>();
	UJGOverworldInteractionComponent* InteractionComponent = NewObject<UJGOverworldInteractionComponent>(SourceActor);

	TestNotNull(TEXT("Interaction component created"), InteractionComponent);
	if (!Interactable || !InteractionComponent)
	{
		return false;
	}

	TestTrue(TEXT("Direct interaction succeeds with valid source and target"), InteractionComponent->TryInteractActor(Interactable));

	Interactable->bInteractionEnabled = false;
	TestFalse(TEXT("Direct interaction fails when interactable is disabled"), InteractionComponent->TryInteractActor(Interactable));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJGMapTransitionTriggerBuildRequestTest,
	"JourneyGame.Overworld.Transition.TriggerBuildRequest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FJGMapTransitionTriggerBuildRequestTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	AJGMapTransitionTriggerBase* Trigger = NewObject<AJGMapTransitionTriggerBase>();
	AActor* TriggeringActor = NewObject<AActor>();

	TestNotNull(TEXT("Transition trigger created"), Trigger);
	if (!Trigger)
	{
		return false;
	}

	Trigger->DestinationMapId = TEXT("Dungeon01");
	Trigger->DestinationSpawnTag = TEXT("Door_A");
	Trigger->bSeamlessTravel = true;
	Trigger->DebugReason = TEXT("Test transition");

	const FJGMapTransitionRequest Request = Trigger->BuildTransitionRequest();
	TestEqual(TEXT("Transition request map id"), Request.MapId, FName(TEXT("Dungeon01")));
	TestEqual(TEXT("Transition request spawn tag"), Request.SpawnTag, FName(TEXT("Door_A")));
	TestTrue(TEXT("Transition request preserves seamless flag"), Request.bSeamless);
	TestEqual(TEXT("Transition request preserves debug reason"), Request.DebugReason, FString(TEXT("Test transition")));
	TestTrue(TEXT("Trigger accepts valid actors when configured"), Trigger->CanTriggerTransition(TriggeringActor));

	Trigger->bOneShot = true;
	Trigger->bConsumed = true;
	TestFalse(TEXT("Consumed one-shot trigger rejects future transitions"), Trigger->CanTriggerTransition(TriggeringActor));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJGEncounterZoneConsumptionTest,
	"JourneyGame.Overworld.Encounter.ZoneConsumption",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FJGEncounterZoneConsumptionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	AJGEncounterZoneBase* EncounterZone = NewObject<AJGEncounterZoneBase>();
	UJGEncounterTableDataAsset* EncounterTable = NewObject<UJGEncounterTableDataAsset>();
	AActor* TriggeringActor = NewObject<AActor>();

	TestNotNull(TEXT("Encounter zone created"), EncounterZone);
	if (!EncounterZone || !EncounterTable)
	{
		return false;
	}

	EncounterZone->EncounterTable = EncounterTable;
	TestTrue(TEXT("Encounter zone accepts a valid trigger actor when enabled"), EncounterZone->CanTriggerEncounter_Implementation(TriggeringActor));

	EncounterZone->bOneShot = true;
	EncounterZone->HandleEncounterConsumed_Implementation(TriggeringActor);
	TestTrue(TEXT("One-shot encounter zone is consumed after handling"), EncounterZone->bConsumed);
	TestFalse(TEXT("Consumed encounter zone rejects another trigger"), EncounterZone->CanTriggerEncounter_Implementation(TriggeringActor));

	EncounterZone->bEnabled = false;
	EncounterZone->bConsumed = false;
	TestFalse(TEXT("Disabled encounter zone rejects trigger"), EncounterZone->CanTriggerEncounter_Implementation(TriggeringActor));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJGOverworldSubsystemTransitionLifecycleTest,
	"JourneyGame.Overworld.Subsystem.TransitionLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FJGOverworldSubsystemTransitionLifecycleTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UJGOverworldSubsystem* OverworldSubsystem = NewObject<UJGOverworldSubsystem>(GameInstance);
	TestNotNull(TEXT("Overworld subsystem created"), OverworldSubsystem);
	if (!OverworldSubsystem)
	{
		return false;
	}

	FJGMapTransitionRequest InvalidRequest;
	TestFalse(TEXT("Invalid transition request is rejected"), OverworldSubsystem->RequestMapTransition(InvalidRequest));

	FJGMapTransitionRequest ValidRequest;
	ValidRequest.MapId = TEXT("Town");
	ValidRequest.SpawnTag = TEXT("Spawn_Main");
	ValidRequest.bSeamless = true;
	TestTrue(TEXT("Valid transition request is accepted"), OverworldSubsystem->RequestMapTransition(ValidRequest));
	TestTrue(TEXT("Pending transition flag is set"), OverworldSubsystem->HasPendingMapTransition());
	TestEqual(TEXT("Next spawn tag is propagated"), OverworldSubsystem->GetNextSpawnTag(), FName(TEXT("Spawn_Main")));
	TestFalse(TEXT("Duplicate transition request is rejected while pending"), OverworldSubsystem->RequestMapTransition(ValidRequest));

	FJGMapTransitionRequest ConsumedRequest;
	TestTrue(TEXT("Pending transition can be consumed"), OverworldSubsystem->ConsumePendingMapTransition(ConsumedRequest));
	TestEqual(TEXT("Consumed transition keeps map id"), ConsumedRequest.MapId, FName(TEXT("Town")));
	TestEqual(TEXT("Consumed transition status is marked"), ConsumedRequest.Status, EJGTransitionRequestStatus::Consumed);
	TestFalse(TEXT("No pending transition remains after consume"), OverworldSubsystem->HasPendingMapTransition());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJGOverworldSubsystemEncounterLifecycleTest,
	"JourneyGame.Overworld.Subsystem.EncounterLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FJGOverworldSubsystemEncounterLifecycleTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UJGOverworldSubsystem* OverworldSubsystem = NewObject<UJGOverworldSubsystem>(GameInstance);
	TestNotNull(TEXT("Overworld subsystem created"), OverworldSubsystem);
	if (!OverworldSubsystem)
	{
		return false;
	}

	FJGEncounterRequest InvalidRequest;
	TestFalse(TEXT("Encounter request without table is rejected"), OverworldSubsystem->RequestEncounter(InvalidRequest));

	FJGEncounterRequest ValidRequest;
	ValidRequest.SourceActor = NewObject<AActor>();
	ValidRequest.TriggeringActor = NewObject<AActor>();
	ValidRequest.EncounterTable = NewObject<UJGEncounterTableDataAsset>();
	TestTrue(TEXT("Encounter request with table is accepted"), OverworldSubsystem->RequestEncounter(ValidRequest));
	TestTrue(TEXT("Pending encounter flag is set"), OverworldSubsystem->HasPendingEncounter());
	TestFalse(TEXT("Duplicate encounter request is rejected while pending"), OverworldSubsystem->RequestEncounter(ValidRequest));

	FJGEncounterRequest ConsumedRequest;
	TestTrue(TEXT("Pending encounter can be consumed"), OverworldSubsystem->ConsumePendingEncounter(ConsumedRequest));
	TestEqual(TEXT("Consumed encounter preserves encounter table"), ConsumedRequest.EncounterTable, ValidRequest.EncounterTable);
	TestFalse(TEXT("No pending encounter remains after consume"), OverworldSubsystem->HasPendingEncounter());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJGOverworldCharacterStepMoveLifecycleTest,
	"JourneyGame.Overworld.Movement.StepLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FJGOverworldCharacterStepMoveLifecycleTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	AJGOverworldCharacterBase* Character = NewObject<AJGOverworldCharacterBase>();
	TestNotNull(TEXT("Overworld character created"), Character);
	if (!Character)
	{
		return false;
	}

	Character->GridSettings.TileSizeUU = 32.0f;
	Character->GridSettings.StepDuration = 0.2f;
	Character->SetActorLocation(FVector::ZeroVector);

	TestTrue(TEXT("Move request succeeds from idle state"), Character->RequestMoveRight());
	TestTrue(TEXT("Step move enters in-progress state"), Character->IsStepMovementInProgress());
	TestFalse(TEXT("Second move request is rejected while moving"), Character->RequestMoveUp());

	Character->Tick(0.1f);
	TestEqual(TEXT("Half-step moves halfway across one tile"), Character->GetActorLocation(), FVector(16.0f, 0.0f, 0.0f));

	Character->Tick(0.1f);
	TestFalse(TEXT("Step move completes after full duration"), Character->IsStepMovementInProgress());
	TestEqual(TEXT("Final location lands on next tile"), Character->GetActorLocation(), FVector(32.0f, 0.0f, 0.0f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJGOverworldCharacterStepMoveValidationTest,
	"JourneyGame.Overworld.Movement.Validation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FJGOverworldCharacterStepMoveValidationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	AJGOverworldCharacterBase* Character = NewObject<AJGOverworldCharacterBase>();
	TestNotNull(TEXT("Overworld character created"), Character);
	if (!Character)
	{
		return false;
	}

	Character->GridSettings.TileSizeUU = 32.0f;
	Character->GridSettings.StepDuration = 0.2f;
	Character->bMovementEnabled = false;
	TestFalse(TEXT("Disabled movement rejects requests"), Character->RequestMoveUp());

	Character->bMovementEnabled = true;
	TestFalse(TEXT("Zero direction is rejected"), Character->RequestMoveInDirection(FVector2D::ZeroVector));

	Character->SetActorLocation(FVector(17.0f, 14.0f, 3.0f));
	TestTrue(TEXT("Valid move request succeeds after resnap"), Character->RequestMoveLeft());
	Character->Tick(0.2f);
	TestEqual(TEXT("Completed move snaps to exact grid"), Character->GetActorLocation(), FVector(-32.0f, 0.0f, 3.0f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FJGOverworldGameModeDefaultClassTest,
	"JourneyGame.Overworld.GameMode.DefaultClasses",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FJGOverworldGameModeDefaultClassTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	AJGOverworldGameModeBase* GameMode = NewObject<AJGOverworldGameModeBase>();
	TestNotNull(TEXT("Overworld game mode created"), GameMode);
	if (!GameMode)
	{
		return false;
	}

	TestTrue(TEXT("Default pawn class is configured"), GameMode->DefaultPawnClass != nullptr);
	TestTrue(TEXT("Player controller class is configured"), GameMode->PlayerControllerClass != nullptr);
	TestTrue(TEXT("Default pawn class stays on the native overworld chain"), GameMode->DefaultPawnClass->IsChildOf(AJGOverworldCharacterBase::StaticClass()));
	TestTrue(TEXT("Player controller class stays on the native overworld chain"), GameMode->PlayerControllerClass->IsChildOf(AJGOverworldPlayerControllerBase::StaticClass()));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
