#include "Battle/JGBattleSimulation.h"

#include "Battle/JGBattleDice.h"
#include "Battle/Data/JGBattleEncounterDefinitionDataAsset.h"
#include "Battle/Data/JGBattleUnitArchetypeDataAsset.h"
#include "Core/JGLog.h"
#include "Containers/Queue.h"

namespace JGBattleSimInternal
{
	static void AddEvent(FJGBattleCommandResult& Result, EJGBattleEventType Type, const FText& Message = FText::GetEmpty())
	{
		FJGBattleEvent& Event = Result.Events.AddDefaulted_GetRef();
		Event.Type = Type;
		Event.Message = Message;
	}

	static int32 GridDistance(const FJGBattleGridCoord& A, const FJGBattleGridCoord& B)
	{
		return FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y);
	}

	static bool IsInsideGrid(const FJGBattleState& State, const FJGBattleGridCoord& Coord)
	{
		return Coord.X >= 0 && Coord.X < State.GridWidth && Coord.Y >= 0 && Coord.Y < State.GridHeight;
	}

	static bool IsBlocked(const FJGBattleState& State, const FJGBattleGridCoord& Coord)
	{
		return State.BlockedTiles.Contains(Coord);
	}

	static int32 FindUnitIndexByHandle(const FJGBattleState& State, const FJGBattleUnitHandle Handle)
	{
		for (int32 i = 0; i < State.Units.Num(); ++i)
		{
			if (State.Units[i].Handle == Handle)
			{
				return i;
			}
		}
		return INDEX_NONE;
	}

	static FJGBattleUnitState* FindUnitMutable(FJGBattleState& State, const FJGBattleUnitHandle Handle)
	{
		const int32 Index = FindUnitIndexByHandle(State, Handle);
		return Index != INDEX_NONE ? &State.Units[Index] : nullptr;
	}

	static const FJGBattleUnitState* FindUnit(const FJGBattleState& State, const FJGBattleUnitHandle Handle)
	{
		const int32 Index = FindUnitIndexByHandle(State, Handle);
		return Index != INDEX_NONE ? &State.Units[Index] : nullptr;
	}

	static bool IsOccupied(const FJGBattleState& State, const FJGBattleGridCoord& Coord, const FJGBattleUnitHandle Ignore = {})
	{
		for (const FJGBattleUnitState& Unit : State.Units)
		{
			if (!Unit.IsAlive())
			{
				continue;
			}
			if (Ignore.IsValid() && Unit.Handle == Ignore)
			{
				continue;
			}
			if (Unit.Pos == Coord)
			{
				return true;
			}
		}
		return false;
	}

	static FJGBattleUnitState* GetCurrentUnitMutable(FJGBattleState& State)
	{
		if (State.TurnOrder.Num() == 0 || !State.TurnOrder.IsValidIndex(State.CurrentTurnIndex))
		{
			return nullptr;
		}
		return FindUnitMutable(State, State.TurnOrder[State.CurrentTurnIndex]);
	}

	static const FJGBattleUnitState* GetCurrentUnit(const FJGBattleState& State)
	{
		if (State.TurnOrder.Num() == 0 || !State.TurnOrder.IsValidIndex(State.CurrentTurnIndex))
		{
			return nullptr;
		}
		return FindUnit(State, State.TurnOrder[State.CurrentTurnIndex]);
	}

	static bool IsPlayersTurn(const FJGBattleState& State)
	{
		const FJGBattleUnitState* Current = GetCurrentUnit(State);
		return Current && Current->Team == EJGBattleTeam::Player && Current->IsAlive();
	}

	static void PrepareUnitForNewTurn(FJGBattleUnitState& Unit)
	{
		Unit.MoveRemaining = Unit.MoveRange;
		Unit.bActionAvailable = true;
		Unit.bBonusAvailable = true;
		Unit.bReactionAvailable = true;
		Unit.bDisengageUntilTurnEnd = false;
		Unit.ACBonus = 0;
		Unit.AttackBonusBuff = 0;
	}

	static TArray<FJGBattleGridCoord> ComputeReachableTiles(const FJGBattleState& State, const FJGBattleUnitState& Unit)
	{
		TArray<FJGBattleGridCoord> Result;
		if (!Unit.IsAlive() || Unit.MoveRemaining <= 0)
		{
			return Result;
		}

		TQueue<FJGBattleGridCoord> Queue;
		TMap<FJGBattleGridCoord, int32> Distance;
		Queue.Enqueue(Unit.Pos);
		Distance.Add(Unit.Pos, 0);

		const FJGBattleGridCoord Dirs[4] = {
			FJGBattleGridCoord(1, 0), FJGBattleGridCoord(-1, 0), FJGBattleGridCoord(0, 1), FJGBattleGridCoord(0, -1)
		};

		while (!Queue.IsEmpty())
		{
			FJGBattleGridCoord Current;
			Queue.Dequeue(Current);
			const int32 CurrentDist = Distance[Current];

			for (const FJGBattleGridCoord& Dir : Dirs)
			{
				const FJGBattleGridCoord Next(Current.X + Dir.X, Current.Y + Dir.Y);
				const int32 NextDist = CurrentDist + 1;
				if (NextDist > Unit.MoveRemaining || !IsInsideGrid(State, Next) || IsBlocked(State, Next))
				{
					continue;
				}
				if (IsOccupied(State, Next, Unit.Handle))
				{
					continue;
				}
				if (Distance.Contains(Next))
				{
					continue;
				}
				Distance.Add(Next, NextDist);
				Queue.Enqueue(Next);
				Result.Add(Next);
			}
		}

		return Result;
	}

	static TArray<FJGBattleUnitHandle> GetOpportunityAttackers(const FJGBattleState& State, const FJGBattleUnitState& Mover, const FJGBattleGridCoord& From, const FJGBattleGridCoord& To)
	{
		TArray<FJGBattleUnitHandle> Provokers;
		for (const FJGBattleUnitState& Unit : State.Units)
		{
			if (!Unit.IsAlive() || Unit.Team == Mover.Team || !Unit.bReactionAvailable)
			{
				continue;
			}
			const bool bWasAdjacent = GridDistance(Unit.Pos, From) == 1;
			const bool bStillAdjacent = GridDistance(Unit.Pos, To) == 1;
			if (bWasAdjacent && !bStillAdjacent)
			{
				Provokers.Add(Unit.Handle);
			}
		}
		return Provokers;
	}

	static void PushReachableForSelection(FJGBattleState& State)
	{
		State.ReachableTiles.Reset();
		State.bHasPendingMoveConfirmation = false;
		State.PendingMoveConfirmation = FJGBattlePendingMoveConfirmation{};
		State.OAWarningUnitHandles.Reset();

		if (!State.SelectedPlayerUnit.IsValid())
		{
			return;
		}

		if (const FJGBattleUnitState* Selected = FindUnit(State, State.SelectedPlayerUnit))
		{
			State.ReachableTiles = ComputeReachableTiles(State, *Selected);
		}
	}

	static void SelectIfCurrentPlayer(FJGBattleState& State)
	{
		if (const FJGBattleUnitState* Current = GetCurrentUnit(State))
		{
			if (Current->Team == EJGBattleTeam::Player && Current->IsAlive())
			{
				State.SelectedPlayerUnit = Current->Handle;
				State.bAttackModeActive = false;
				PushReachableForSelection(State);
				return;
			}
		}

		State.SelectedPlayerUnit = FJGBattleUnitHandle{};
		State.bAttackModeActive = false;
		State.ReachableTiles.Reset();
		State.bHasPendingMoveConfirmation = false;
		State.PendingMoveConfirmation = FJGBattlePendingMoveConfirmation{};
		State.OAWarningUnitHandles.Reset();
	}

	static void CheckVictory(FJGBattleState& State, FJGBattleCommandResult& Result)
	{
		bool bAnyPlayersAlive = false;
		bool bAnyEnemiesAlive = false;
		for (const FJGBattleUnitState& Unit : State.Units)
		{
			if (!Unit.IsAlive())
			{
				continue;
			}
			bAnyPlayersAlive |= (Unit.Team == EJGBattleTeam::Player);
			bAnyEnemiesAlive |= (Unit.Team == EJGBattleTeam::Enemy);
		}

		if (!bAnyEnemiesAlive || !bAnyPlayersAlive)
		{
			State.bCombatEnded = true;
			State.WinningTeam = bAnyEnemiesAlive ? EJGBattleTeam::Enemy : EJGBattleTeam::Player;
			AddEvent(Result, EJGBattleEventType::Victory, FText::FromString(State.WinningTeam == EJGBattleTeam::Player ? TEXT("Players win") : TEXT("Enemies win")));
		}
	}

	static void ResolveAttack(FJGBattleState& State, FJGBattleUnitState& Attacker, FJGBattleUnitState& Target, FRandomStream& Stream, FJGBattleCommandResult& Result, bool bConsumesAction)
	{
		const int32 AttackRoll = FJGBattleDice::RollD20(Stream);
		const int32 AttackTotal = AttackRoll + Attacker.AttackBonus + Attacker.AttackBonusBuff;
		const int32 TargetAC = Target.AC + Target.ACBonus;

		FJGBattleEvent AttackEvent;
		AttackEvent.Type = EJGBattleEventType::AttackResolved;
		AttackEvent.SourceUnit = Attacker.Handle;
		AttackEvent.TargetUnit = Target.Handle;
		AttackEvent.ValueA = AttackRoll;
		AttackEvent.ValueB = AttackTotal;

		const bool bHit = (AttackRoll == 20) || (AttackTotal >= TargetAC);
		if (!bHit)
		{
			AttackEvent.Message = FText::FromString(FString::Printf(TEXT("%s misses %s"), *Attacker.UnitId.ToString(), *Target.UnitId.ToString()));
			Result.Events.Add(AttackEvent);
			Attacker.AttackBonusBuff = 0;
			if (bConsumesAction)
			{
				Attacker.bActionAvailable = false;
			}
			return;
		}

		FJGDiceRollSpec Spec;
		if (!FJGBattleDice::ParseDiceNotation(Attacker.DamageDice, Spec))
		{
			Spec = FJGDiceRollSpec{};
		}
		const FJGDiceRollResult BaseDamage = FJGBattleDice::Roll(Spec, Stream);
		int32 Damage = BaseDamage.Total;
		if (AttackRoll == 20)
		{
			Damage += FJGBattleDice::Roll(Spec, Stream).Total;
		}

		Target.Hp -= Damage;
		AttackEvent.Message = FText::FromString(FString::Printf(TEXT("%s %s %s for %d"),
			*Attacker.UnitId.ToString(),
			AttackRoll == 20 ? TEXT("crits") : TEXT("hits"),
			*Target.UnitId.ToString(),
			Damage));
		Result.Events.Add(AttackEvent);

		Attacker.AttackBonusBuff = 0;
		if (bConsumesAction)
		{
			Attacker.bActionAvailable = false;
		}

		if (Target.Hp <= 0)
		{
			FJGBattleEvent Defeat;
			Defeat.Type = EJGBattleEventType::UnitDefeated;
			Defeat.TargetUnit = Target.Handle;
			Defeat.Message = FText::FromString(FString::Printf(TEXT("%s is defeated"), *Target.UnitId.ToString()));
			Result.Events.Add(Defeat);
		}
	}

	static void ResolveOpportunityAttacksIfNeeded(FJGBattleState& State, FJGBattleUnitState& Mover, const FJGBattleGridCoord& From, const FJGBattleGridCoord& To, FRandomStream& Stream, FJGBattleCommandResult& Result)
	{
		if (Mover.bDisengageUntilTurnEnd)
		{
			return;
		}

		TArray<FJGBattleUnitHandle> Provokers = GetOpportunityAttackers(State, Mover, From, To);
		for (const FJGBattleUnitHandle ProvokerHandle : Provokers)
		{
			FJGBattleUnitState* Attacker = FindUnitMutable(State, ProvokerHandle);
			if (!Attacker || !Attacker->IsAlive() || !Attacker->bReactionAvailable || !Mover.IsAlive())
			{
				continue;
			}

			Attacker->bReactionAvailable = false;
			FJGBattleEvent OAEvent;
			OAEvent.Type = EJGBattleEventType::OpportunityAttackTriggered;
			OAEvent.SourceUnit = Attacker->Handle;
			OAEvent.TargetUnit = Mover.Handle;
			OAEvent.Message = FText::FromString(FString::Printf(TEXT("%s makes opportunity attack on %s"), *Attacker->UnitId.ToString(), *Mover.UnitId.ToString()));
			Result.Events.Add(OAEvent);

			ResolveAttack(State, *Attacker, Mover, Stream, Result, false);
		}
	}

	static void ExecuteMove(FJGBattleState& State, FJGBattleUnitState& Unit, const FJGBattleGridCoord& Target, int32 Cost, FRandomStream& Stream, FJGBattleCommandResult& Result)
	{
		const FJGBattleGridCoord From = Unit.Pos;
		ResolveOpportunityAttacksIfNeeded(State, Unit, From, Target, Stream, Result);
		if (!Unit.IsAlive())
		{
			CheckVictory(State, Result);
			return;
		}

		Unit.Pos = Target;
		Unit.MoveRemaining = FMath::Max(0, Unit.MoveRemaining - Cost);

		FJGBattleEvent Event;
		Event.Type = EJGBattleEventType::UnitMoved;
		Event.SourceUnit = Unit.Handle;
		Event.Coord = Target;
		Event.ValueA = Cost;
		Event.Message = FText::FromString(FString::Printf(TEXT("%s moved to (%d,%d)"), *Unit.UnitId.ToString(), Target.X, Target.Y));
		Result.Events.Add(Event);

		if (Unit.Team == EJGBattleTeam::Player && State.SelectedPlayerUnit == Unit.Handle)
		{
			State.ReachableTiles = ComputeReachableTiles(State, Unit);
		}
	}

	static void AdvanceToNextLivingUnit(FJGBattleState& State, FJGBattleCommandResult& Result)
	{
		if (State.TurnOrder.Num() == 0)
		{
			return;
		}
		for (int32 Scan = 0; Scan < State.TurnOrder.Num(); ++Scan)
		{
			State.CurrentTurnIndex = (State.CurrentTurnIndex + 1) % State.TurnOrder.Num();
			if (FJGBattleUnitState* Unit = GetCurrentUnitMutable(State))
			{
				if (!Unit->IsAlive())
				{
					continue;
				}
				PrepareUnitForNewTurn(*Unit);
				SelectIfCurrentPlayer(State);

				FJGBattleEvent TurnStart;
				TurnStart.Type = EJGBattleEventType::TurnStarted;
				TurnStart.SourceUnit = Unit->Handle;
				TurnStart.Message = FText::FromString(FString::Printf(TEXT("Turn: %s"), *Unit->UnitId.ToString()));
				Result.Events.Add(TurnStart);
				return;
			}
		}
	}

	static bool CanAttackTarget(const FJGBattleUnitState& Attacker, const FJGBattleUnitState& Target)
	{
		return Attacker.IsAlive() && Target.IsAlive() && GridDistance(Attacker.Pos, Target.Pos) <= Attacker.AttackRange;
	}

	static FJGBattleUnitState* FindNearestEnemyTarget(FJGBattleState& State, const FJGBattleUnitState& Enemy)
	{
		FJGBattleUnitState* Closest = nullptr;
		int32 Best = MAX_int32;
		for (FJGBattleUnitState& Unit : State.Units)
		{
			if (!Unit.IsAlive() || Unit.Team == Enemy.Team)
			{
				continue;
			}
			const int32 Dist = GridDistance(Enemy.Pos, Unit.Pos);
			if (Dist < Best)
			{
				Best = Dist;
				Closest = &Unit;
			}
		}
		return Closest;
	}

	static TOptional<FJGBattleGridCoord> ChooseEnemyStep(const FJGBattleState& State, const FJGBattleUnitState& Enemy, const FJGBattleGridCoord& Target)
	{
		const FJGBattleGridCoord OrderedDirs[4] = {
			FJGBattleGridCoord(1, 0), FJGBattleGridCoord(-1, 0), FJGBattleGridCoord(0, 1), FJGBattleGridCoord(0, -1)
		};

		bool bFound = false;
		FJGBattleGridCoord Best;
		int32 BestDist = MAX_int32;
		for (const FJGBattleGridCoord& Dir : OrderedDirs)
		{
			const FJGBattleGridCoord Candidate(Enemy.Pos.X + Dir.X, Enemy.Pos.Y + Dir.Y);
			if (!IsInsideGrid(State, Candidate) || IsBlocked(State, Candidate) || IsOccupied(State, Candidate, Enemy.Handle))
			{
				continue;
			}
			const int32 Dist = GridDistance(Candidate, Target);
			if (!bFound || Dist < BestDist)
			{
				bFound = true;
				Best = Candidate;
				BestDist = Dist;
			}
		}
		if (!bFound)
		{
			return {};
		}
		return Best;
	}
}

bool FJGBattleSimulation::InitializeFromEncounter(UJGBattleEncounterDefinitionDataAsset* EncounterDefinition, FRandomStream& Stream, FJGBattleState& OutState, FJGBattleCommandResult& OutResult)
{
	OutState = FJGBattleState{};
	OutResult = FJGBattleCommandResult{};

	if (!EncounterDefinition)
	{
		OutResult.FailureReason = EJGBattleFailureReason::InvalidState;
		return false;
	}
	FString ValidationError;
	if (!EncounterDefinition->ValidateDefinition(ValidationError))
	{
		OutResult.FailureReason = EJGBattleFailureReason::InvalidState;
		FJGBattleEvent Event;
		Event.Type = EJGBattleEventType::Message;
		Event.Message = FText::FromString(ValidationError);
		OutResult.Events.Add(Event);
		return false;
	}

	OutState.SourceEncounterDefinition = EncounterDefinition;
	OutState.GridWidth = EncounterDefinition->GridWidth;
	OutState.GridHeight = EncounterDefinition->GridHeight;
	OutState.BlockedTiles = EncounterDefinition->Blockers;
	OutState.Units.Reset();
	OutState.TurnOrder.Reset();
	OutState.InitiativeTotalsByHandle.Reset();

	int32 NextHandle = 0;
	auto AddSlots = [&](const TArray<FJGBattleEncounterSlot>& Slots, EJGBattleTeam Team)
	{
		for (int32 i = 0; i < Slots.Num(); ++i)
		{
			const FJGBattleEncounterSlot& Slot = Slots[i];
			const UJGBattleUnitArchetypeDataAsset* Arch = Slot.Archetype;
			if (!Arch)
			{
				continue;
			}
			FJGBattleUnitState& Unit = OutState.Units.AddDefaulted_GetRef();
			Unit.Handle.Value = NextHandle++;
			Unit.UnitId = Slot.UnitIdOverride.IsNone() ? FName(FString::Printf(TEXT("%s-%d"), Team == EJGBattleTeam::Player ? TEXT("p") : TEXT("e"), i + 1)) : Slot.UnitIdOverride;
			Unit.Team = Team;
			Unit.Role = Arch->Role;
			Unit.Pos = Slot.SpawnCoord;
			Unit.MaxHp = Arch->MaxHp;
			Unit.Hp = Arch->MaxHp;
			Unit.AC = Arch->AC;
			Unit.AttackBonus = Arch->AttackBonus;
			Unit.InitiativeBonus = Arch->InitiativeBonus;
			Unit.DamageDice = Arch->DamageDice;
			Unit.MoveRange = Arch->MoveRange;
			Unit.MoveRemaining = Arch->MoveRange;
			Unit.AttackRange = Arch->AttackRange;
			Unit.BonusAbilityType = Arch->BonusAbilityType;
			Unit.BonusAbilityLabel = Arch->BonusAbilityLabel;
			Unit.Tags = Arch->Tags;
		}
	};

	AddSlots(EncounterDefinition->PlayerSlots, EJGBattleTeam::Player);
	AddSlots(EncounterDefinition->EnemySlots, EJGBattleTeam::Enemy);

	for (const FJGBattleUnitState& Unit : OutState.Units)
	{
		const int32 Total = FJGBattleDice::RollD4(Stream) + Unit.InitiativeBonus;
		OutState.InitiativeTotalsByHandle.Add(Unit.Handle.Value, Total);
		OutState.TurnOrder.Add(Unit.Handle);
	}

	OutState.TurnOrder.Sort([&](const FJGBattleUnitHandle& A, const FJGBattleUnitHandle& B)
	{
		const FJGBattleUnitState* UnitA = JGBattleSimInternal::FindUnit(OutState, A);
		const FJGBattleUnitState* UnitB = JGBattleSimInternal::FindUnit(OutState, B);
		const int32 TotalA = OutState.InitiativeTotalsByHandle.FindRef(A.Value);
		const int32 TotalB = OutState.InitiativeTotalsByHandle.FindRef(B.Value);
		if (TotalA != TotalB) return TotalA > TotalB;
		if (UnitA && UnitB && UnitA->InitiativeBonus != UnitB->InitiativeBonus) return UnitA->InitiativeBonus > UnitB->InitiativeBonus;
		if (UnitA && UnitB && UnitA->Team != UnitB->Team) return UnitA->Team == EJGBattleTeam::Player;
		const FString AId = UnitA ? UnitA->UnitId.ToString() : FString();
		const FString BId = UnitB ? UnitB->UnitId.ToString() : FString();
		return AId < BId;
	});

	OutState.CurrentTurnIndex = 0;
	if (FJGBattleUnitState* Current = JGBattleSimInternal::GetCurrentUnitMutable(OutState))
	{
		JGBattleSimInternal::PrepareUnitForNewTurn(*Current);
	}
	JGBattleSimInternal::SelectIfCurrentPlayer(OutState);

	OutResult.bSuccess = true;
	JGBattleSimInternal::AddEvent(OutResult, EJGBattleEventType::CombatStarted, FText::FromString(TEXT("Combat started")));
	if (const FJGBattleUnitState* Current = JGBattleSimInternal::GetCurrentUnit(OutState))
	{
		FJGBattleEvent Turn;
		Turn.Type = EJGBattleEventType::TurnStarted;
		Turn.SourceUnit = Current->Handle;
		Turn.Message = FText::FromString(FString::Printf(TEXT("Turn: %s"), *Current->UnitId.ToString()));
		OutResult.Events.Add(Turn);
	}
	return true;
}

FJGBattleCommandResult FJGBattleSimulation::ExecuteCommand(FJGBattleState& State, const FJGBattleCommand& Command, FRandomStream& Stream)
{
	using namespace JGBattleSimInternal;
	FJGBattleCommandResult Result;

	if (State.bCombatEnded)
	{
		Result.FailureReason = EJGBattleFailureReason::InvalidState;
		return Result;
	}

	auto Fail = [&](EJGBattleFailureReason Reason, const TCHAR* Msg = nullptr) -> FJGBattleCommandResult
	{
		Result.bSuccess = false;
		Result.FailureReason = Reason;
		if (Msg)
		{
			AddEvent(Result, EJGBattleEventType::Message, FText::FromString(Msg));
		}
		return Result;
	};

	FJGBattleUnitState* Current = GetCurrentUnitMutable(State);
	if (!Current)
	{
		return Fail(EJGBattleFailureReason::InvalidState, TEXT("No current unit"));
	}

	switch (Command.Type)
	{
	case EJGBattleCommandType::SelectUnit:
	{
		if (!IsPlayersTurn(State))
		{
			return Fail(EJGBattleFailureReason::NotPlayersTurn, TEXT("Not player's turn"));
		}
		FJGBattleUnitState* Unit = FindUnitMutable(State, Command.UnitHandle);
		if (!Unit || !Unit->IsAlive() || Unit->Team != EJGBattleTeam::Player || Unit->Handle != Current->Handle)
		{
			return Fail(EJGBattleFailureReason::InvalidUnit, TEXT("Invalid selection"));
		}
		State.SelectedPlayerUnit = Unit->Handle;
		State.bAttackModeActive = false;
		PushReachableForSelection(State);
		Result.bSuccess = true;
		FJGBattleEvent E; E.Type = EJGBattleEventType::UnitSelected; E.SourceUnit = Unit->Handle; E.Message = FText::FromString(TEXT("Unit selected")); Result.Events.Add(E);
		return Result;
	}
	case EJGBattleCommandType::BeginAttack:
	{
		if (!IsPlayersTurn(State)) return Fail(EJGBattleFailureReason::NotPlayersTurn, TEXT("Not player's turn"));
		if (!Current->bActionAvailable) return Fail(EJGBattleFailureReason::NoActionAvailable, TEXT("No action available"));
		State.bAttackModeActive = true;
		Result.bSuccess = true;
		AddEvent(Result, EJGBattleEventType::Message, FText::FromString(TEXT("Attack mode ready")));
		return Result;
	}
	case EJGBattleCommandType::UseDash:
	{
		if (!IsPlayersTurn(State)) return Fail(EJGBattleFailureReason::NotPlayersTurn);
		if (!Current->bActionAvailable) return Fail(EJGBattleFailureReason::NoActionAvailable);
		Current->bActionAvailable = false;
		Current->MoveRemaining += Current->MoveRange;
		State.bAttackModeActive = false;
		if (State.SelectedPlayerUnit == Current->Handle) State.ReachableTiles = ComputeReachableTiles(State, *Current);
		Result.bSuccess = true;
		AddEvent(Result, EJGBattleEventType::Message, FText::FromString(TEXT("Dash used")));
		return Result;
	}
	case EJGBattleCommandType::UseDisengage:
	{
		if (!IsPlayersTurn(State)) return Fail(EJGBattleFailureReason::NotPlayersTurn);
		if (!Current->bActionAvailable) return Fail(EJGBattleFailureReason::NoActionAvailable);
		Current->bActionAvailable = false;
		Current->bDisengageUntilTurnEnd = true;
		State.bAttackModeActive = false;
		Result.bSuccess = true;
		AddEvent(Result, EJGBattleEventType::Message, FText::FromString(TEXT("Disengage used")));
		return Result;
	}
	case EJGBattleCommandType::UseBonusAbility:
	{
		if (!IsPlayersTurn(State)) return Fail(EJGBattleFailureReason::NotPlayersTurn);
		if (!Current->bBonusAvailable) return Fail(EJGBattleFailureReason::NoBonusAvailable);
		Current->bBonusAvailable = false;
		switch (Current->BonusAbilityType)
		{
		case EJGBattleBonusAbilityType::Brace: Current->ACBonus += 2; break;
		case EJGBattleBonusAbilityType::Focus: Current->AttackBonusBuff += 2; break;
		case EJGBattleBonusAbilityType::Quickstep: Current->MoveRemaining += 2; if (State.SelectedPlayerUnit == Current->Handle) State.ReachableTiles = ComputeReachableTiles(State, *Current); break;
		default: break;
		}
		Result.bSuccess = true;
		AddEvent(Result, EJGBattleEventType::Message, FText::FromString(TEXT("Bonus ability used")));
		return Result;
	}
	case EJGBattleCommandType::Move:
	case EJGBattleCommandType::ConfirmMove:
	{
		if (!IsPlayersTurn(State)) return Fail(EJGBattleFailureReason::NotPlayersTurn);
		if (!State.SelectedPlayerUnit.IsValid() || State.SelectedPlayerUnit != Current->Handle) return Fail(EJGBattleFailureReason::InvalidUnit);
		const FJGBattleGridCoord Target = Command.TargetCoord;
		if (!IsInsideGrid(State, Target)) return Fail(EJGBattleFailureReason::TileBlocked);
		if (IsBlocked(State, Target)) return Fail(EJGBattleFailureReason::TileBlocked);
		if (IsOccupied(State, Target, Current->Handle)) return Fail(EJGBattleFailureReason::TileOccupied);

		const int32 DistanceCost = GridDistance(Current->Pos, Target);
		if (DistanceCost <= 0) return Fail(EJGBattleFailureReason::InvalidTarget);
		if (DistanceCost > Current->MoveRemaining) return Fail(EJGBattleFailureReason::NotEnoughMovement);

		if (!State.ReachableTiles.Contains(Target))
		{
			return Fail(EJGBattleFailureReason::TileUnreachable);
		}

		const TArray<FJGBattleUnitHandle> Provokers = Current->bDisengageUntilTurnEnd ? TArray<FJGBattleUnitHandle>() : GetOpportunityAttackers(State, *Current, Current->Pos, Target);
		const bool bNeedsConfirm = Provokers.Num() > 0;

		if (Command.Type == EJGBattleCommandType::Move && bNeedsConfirm)
		{
			State.bHasPendingMoveConfirmation = true;
			State.PendingMoveConfirmation.UnitHandle = Current->Handle;
			State.PendingMoveConfirmation.Target = Target;
			State.PendingMoveConfirmation.ProvokerHandles = Provokers;
			State.OAWarningUnitHandles = Provokers;
			Result.bSuccess = false;
			Result.FailureReason = EJGBattleFailureReason::MoveRequiresConfirmation;
			Result.bHasPendingMoveConfirmation = true;
			Result.PendingMoveConfirmation = State.PendingMoveConfirmation;
			AddEvent(Result, EJGBattleEventType::MoveConfirmationRequired, FText::FromString(TEXT("Move requires confirmation (opportunity attack)")));
			return Result;
		}

		if (Command.Type == EJGBattleCommandType::ConfirmMove)
		{
			if (!State.bHasPendingMoveConfirmation)
			{
				return Fail(EJGBattleFailureReason::NoPendingMoveConfirmation);
			}
			if (!(State.PendingMoveConfirmation.UnitHandle == Current->Handle && State.PendingMoveConfirmation.Target == Target))
			{
				return Fail(EJGBattleFailureReason::NoPendingMoveConfirmation);
			}
		}

		State.bHasPendingMoveConfirmation = false;
		State.PendingMoveConfirmation = FJGBattlePendingMoveConfirmation{};
		State.OAWarningUnitHandles.Reset();
		ExecuteMove(State, *Current, Target, DistanceCost, Stream, Result);
		Result.bSuccess = true;
		return Result;
	}
	case EJGBattleCommandType::Attack:
	{
		if (!IsPlayersTurn(State)) return Fail(EJGBattleFailureReason::NotPlayersTurn);
		if (!Current->bActionAvailable) return Fail(EJGBattleFailureReason::NoActionAvailable);
		if (!State.SelectedPlayerUnit.IsValid() || State.SelectedPlayerUnit != Current->Handle) return Fail(EJGBattleFailureReason::InvalidUnit);

		FJGBattleUnitState* Target = FindUnitMutable(State, Command.TargetUnitHandle);
		if (!Target || !Target->IsAlive() || Target->Team == Current->Team) return Fail(EJGBattleFailureReason::InvalidTarget);
		if (!CanAttackTarget(*Current, *Target)) return Fail(EJGBattleFailureReason::TargetOutOfRange);

		State.bAttackModeActive = false;
		ResolveAttack(State, *Current, *Target, Stream, Result, true);
		CheckVictory(State, Result);
		Result.bSuccess = true;
		return Result;
	}
	case EJGBattleCommandType::EndTurn:
	{
		if (!IsPlayersTurn(State)) return Fail(EJGBattleFailureReason::NotPlayersTurn);
		FJGBattleEvent TurnEnd; TurnEnd.Type = EJGBattleEventType::TurnEnded; TurnEnd.SourceUnit = Current->Handle; TurnEnd.Message = FText::FromString(TEXT("Turn ended")); Result.Events.Add(TurnEnd);
		State.bAttackModeActive = false;
		AdvanceToNextLivingUnit(State, Result);
		Result.bSuccess = true;
		return Result;
	}
	default:
		return Fail(EJGBattleFailureReason::CommandNotAllowed);
	}
}

FJGBattleCommandResult FJGBattleSimulation::RunEnemyTurnsUntilPlayerInputNeeded(FJGBattleState& State, FRandomStream& Stream)
{
	using namespace JGBattleSimInternal;
	FJGBattleCommandResult Aggregate;

	int32 Safety = 0;
	while (!State.bCombatEnded && !IsPlayersTurn(State) && Safety < State.TurnOrder.Num() + 4)
	{
		FJGBattleUnitState* Enemy = GetCurrentUnitMutable(State);
		if (!Enemy)
		{
			break;
		}
		if (!Enemy->IsAlive())
		{
			AdvanceToNextLivingUnit(State, Aggregate);
			++Safety;
			continue;
		}

		while (Enemy->MoveRemaining > 0 && Enemy->IsAlive())
		{
			FJGBattleUnitState* Target = FindNearestEnemyTarget(State, *Enemy);
			if (!Target)
			{
				break;
			}
			if (GridDistance(Enemy->Pos, Target->Pos) <= Enemy->AttackRange)
			{
				break;
			}
			TOptional<FJGBattleGridCoord> Step = ChooseEnemyStep(State, *Enemy, Target->Pos);
			if (!Step.IsSet())
			{
				break;
			}
			ExecuteMove(State, *Enemy, Step.GetValue(), 1, Stream, Aggregate);
			CheckVictory(State, Aggregate);
			if (State.bCombatEnded || !Enemy->IsAlive())
			{
				break;
			}
		}

		if (!State.bCombatEnded && Enemy->IsAlive() && Enemy->bActionAvailable)
		{
			if (FJGBattleUnitState* Target = FindNearestEnemyTarget(State, *Enemy))
			{
				if (CanAttackTarget(*Enemy, *Target))
				{
					ResolveAttack(State, *Enemy, *Target, Stream, Aggregate, true);
					CheckVictory(State, Aggregate);
				}
			}
		}

		if (!State.bCombatEnded)
		{
			FJGBattleEvent TurnEnd; TurnEnd.Type = EJGBattleEventType::TurnEnded; TurnEnd.SourceUnit = Enemy->Handle; Aggregate.Events.Add(TurnEnd);
			AdvanceToNextLivingUnit(State, Aggregate);
		}
		++Safety;
	}

	Aggregate.bSuccess = true;
	return Aggregate;
}

bool UJGBattleSimulationBlueprintLibrary::InitializeBattleState(
	UJGBattleEncounterDefinitionDataAsset* EncounterDefinition,
	int32 Seed,
	FJGBattleState& OutState,
	FJGBattleCommandResult& OutResult)
{
	FRandomStream Stream(Seed);
	return FJGBattleSimulation::InitializeFromEncounter(EncounterDefinition, Stream, OutState, OutResult);
}

bool UJGBattleSimulationBlueprintLibrary::ExecuteBattleCommand(
	FJGBattleState& State,
	const FJGBattleCommand& Command,
	int32 Seed,
	int32 StreamAdvance,
	FJGBattleCommandResult& OutResult,
	int32& OutStreamAdvance)
{
	FRandomStream Stream(Seed);
	for (int32 i = 0; i < StreamAdvance; ++i) { Stream.RandHelper(MAX_int32); }
	OutResult = FJGBattleSimulation::ExecuteCommand(State, Command, Stream);
	OutStreamAdvance = StreamAdvance + 1; // coarse tracking for debug harness v1
	return OutResult.bSuccess;
}

bool UJGBattleSimulationBlueprintLibrary::RunEnemyTurnsUntilPlayerInputNeeded(
	FJGBattleState& State,
	int32 Seed,
	int32 StreamAdvance,
	FJGBattleCommandResult& OutResult,
	int32& OutStreamAdvance)
{
	FRandomStream Stream(Seed);
	for (int32 i = 0; i < StreamAdvance; ++i) { Stream.RandHelper(MAX_int32); }
	OutResult = FJGBattleSimulation::RunEnemyTurnsUntilPlayerInputNeeded(State, Stream);
	OutStreamAdvance = StreamAdvance + 1;
	return OutResult.bSuccess;
}

FJGBattleDebugSnapshot UJGBattleSimulationBlueprintLibrary::BuildDebugSnapshot(const FJGBattleState& State)
{
	FJGBattleDebugSnapshot Snapshot;
	Snapshot.State = State;

	for (const FJGBattleUnitState& Unit : State.Units)
	{
		FJGBattleDebugUnitView& View = Snapshot.Units.AddDefaulted_GetRef();
		View.Handle = Unit.Handle;
		View.UnitId = Unit.UnitId;
		View.Team = Unit.Team;
		View.Pos = Unit.Pos;
		View.Hp = Unit.Hp;
		View.MaxHp = Unit.MaxHp;
	}

	for (int32 X = 0; X < State.GridWidth; ++X)
	{
		for (int32 Y = 0; Y < State.GridHeight; ++Y)
		{
			FJGBattleDebugTileView& Tile = Snapshot.Tiles.AddDefaulted_GetRef();
			Tile.Coord = FJGBattleGridCoord(X, Y);
			Tile.bBlocked = State.BlockedTiles.Contains(Tile.Coord);
			Tile.bReachable = State.ReachableTiles.Contains(Tile.Coord);
		}
	}

	return Snapshot;
}
