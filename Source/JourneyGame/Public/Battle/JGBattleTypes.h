#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "JGBattleTypes.generated.h"

class UJGBattleEncounterDefinitionDataAsset;

UENUM(BlueprintType)
enum class EJGBattleTeam : uint8
{
	Player,
	Enemy
};

UENUM(BlueprintType)
enum class EJGBattleUnitRole : uint8
{
	Guardian,
	Striker,
	Scout,
	Brute,
	Raider,
	Arbalist
};

UENUM(BlueprintType)
enum class EJGBattleBonusAbilityType : uint8
{
	None,
	Brace,
	Focus,
	Quickstep
};

UENUM(BlueprintType)
enum class EJGBattleCommandType : uint8
{
	SelectUnit,
	BeginAttack,
	Move,
	ConfirmMove,
	Attack,
	UseDash,
	UseDisengage,
	UseBonusAbility,
	EndTurn
};

UENUM(BlueprintType)
enum class EJGBattleFailureReason : uint8
{
	None,
	InvalidState,
	NotPlayersTurn,
	InvalidUnit,
	InvalidTarget,
	NoActionAvailable,
	NoBonusAvailable,
	NotEnoughMovement,
	TargetOutOfRange,
	TileBlocked,
	TileOccupied,
	TileUnreachable,
	MoveRequiresConfirmation,
	NoPendingMoveConfirmation,
	CommandNotAllowed
};

UENUM(BlueprintType)
enum class EJGBattleEventType : uint8
{
	CombatStarted,
	TurnStarted,
	TurnEnded,
	UnitSelected,
	MoveConfirmationRequired,
	UnitMoved,
	OpportunityAttackTriggered,
	AttackResolved,
	UnitDefeated,
	Victory,
	Message
};

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGBattleGridCoord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 X = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 Y = 0;

	FJGBattleGridCoord() = default;
	FJGBattleGridCoord(int32 InX, int32 InY) : X(InX), Y(InY) {}

	bool operator==(const FJGBattleGridCoord& Other) const { return X == Other.X && Y == Other.Y; }
	friend uint32 GetTypeHash(const FJGBattleGridCoord& Value)
	{
		return HashCombine(::GetTypeHash(Value.X), ::GetTypeHash(Value.Y));
	}
};

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGBattleUnitHandle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 Value = INDEX_NONE;

	bool IsValid() const { return Value != INDEX_NONE; }
	bool operator==(const FJGBattleUnitHandle& Other) const { return Value == Other.Value; }
	friend uint32 GetTypeHash(const FJGBattleUnitHandle& Handle) { return ::GetTypeHash(Handle.Value); }
};

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGBattleMapCell
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FJGBattleGridCoord Coord;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	bool bBlocked = false;
};

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGBattleUnitState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FName UnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FJGBattleUnitHandle Handle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	EJGBattleTeam Team = EJGBattleTeam::Enemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	EJGBattleUnitRole Role = EJGBattleUnitRole::Brute;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FJGBattleGridCoord Pos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 Hp = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 MaxHp = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 AC = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 ACBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 AttackBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 AttackBonusBuff = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 InitiativeBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FString DamageDice = TEXT("1d4");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 MoveRange = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 MoveRemaining = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 AttackRange = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	bool bActionAvailable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	bool bBonusAvailable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	bool bReactionAvailable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	bool bDisengageUntilTurnEnd = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	EJGBattleBonusAbilityType BonusAbilityType = EJGBattleBonusAbilityType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FText BonusAbilityLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FGameplayTagContainer Tags;

	bool IsAlive() const { return Hp > 0; }
};

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGBattlePendingMoveConfirmation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FJGBattleUnitHandle UnitHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FJGBattleGridCoord Target;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	TArray<FJGBattleUnitHandle> ProvokerHandles;

	bool IsValid() const { return UnitHandle.IsValid(); }
};

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGBattleEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	EJGBattleEventType Type = EJGBattleEventType::Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FText Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FJGBattleUnitHandle SourceUnit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FJGBattleUnitHandle TargetUnit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FJGBattleGridCoord Coord;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 ValueA = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 ValueB = 0;
};

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGBattleCommand
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	EJGBattleCommandType Type = EJGBattleCommandType::EndTurn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FJGBattleUnitHandle UnitHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FJGBattleUnitHandle TargetUnitHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FJGBattleGridCoord TargetCoord;
};

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGBattleCommandResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	EJGBattleFailureReason FailureReason = EJGBattleFailureReason::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	TArray<FJGBattleEvent> Events;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	bool bHasPendingMoveConfirmation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FJGBattlePendingMoveConfirmation PendingMoveConfirmation;
};

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGBattleState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 GridWidth = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 GridHeight = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	TArray<FJGBattleGridCoord> BlockedTiles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	TArray<FJGBattleUnitState> Units;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	TArray<FJGBattleUnitHandle> TurnOrder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	TMap<int32, int32> InitiativeTotalsByHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 CurrentTurnIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FJGBattleUnitHandle SelectedPlayerUnit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	TArray<FJGBattleGridCoord> ReachableTiles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	bool bAttackModeActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	bool bHasPendingMoveConfirmation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FJGBattlePendingMoveConfirmation PendingMoveConfirmation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	TArray<FJGBattleUnitHandle> OAWarningUnitHandles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	bool bCombatEnded = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	EJGBattleTeam WinningTeam = EJGBattleTeam::Player;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	TObjectPtr<UJGBattleEncounterDefinitionDataAsset> SourceEncounterDefinition = nullptr;
};

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGBattleDebugUnitView
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FJGBattleUnitHandle Handle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FName UnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	EJGBattleTeam Team = EJGBattleTeam::Enemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FJGBattleGridCoord Pos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 Hp = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	int32 MaxHp = 0;
};

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGBattleDebugTileView
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FJGBattleGridCoord Coord;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	bool bBlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	bool bReachable = false;
};

USTRUCT(BlueprintType)
struct JOURNEYGAME_API FJGBattleDebugSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	FJGBattleState State;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	TArray<FJGBattleDebugUnitView> Units;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
	TArray<FJGBattleDebugTileView> Tiles;
};
