#include "Battle/JGBattleDebugBoardActor.h"

#include "Battle/JGBattleSubsystem.h"
#include "DrawDebugHelpers.h"
#include "Engine/GameInstance.h"

AJGBattleDebugBoardActor::AJGBattleDebugBoardActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AJGBattleDebugBoardActor::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(bRefreshOnTick);
	RefreshFromBattleSubsystem();
}

void AJGBattleDebugBoardActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bRefreshOnTick)
	{
		RefreshFromBattleSubsystem();
	}
}

bool AJGBattleDebugBoardActor::RefreshFromBattleSubsystem()
{
	const UJGBattleSubsystem* BattleSubsystem = ResolveBattleSubsystem();
	if (!BattleSubsystem || !BattleSubsystem->HasActiveBattle() || !GetWorld())
	{
		return false;
	}

	const FJGBattleState State = BattleSubsystem->GetActiveBattleState();
	const FVector TileExtent(TileSizeUU * 0.45f, TileSizeUU * 0.45f, TileDrawHeight);
	const FVector UnitExtent(TileSizeUU * 0.28f, TileSizeUU * 0.28f, UnitDrawHeight);

	for (int32 X = 0; X < State.GridWidth; ++X)
	{
		for (int32 Y = 0; Y < State.GridHeight; ++Y)
		{
			const FJGBattleGridCoord Coord(X, Y);
			FColor Color = FColor::Silver;
			if (State.BlockedTiles.Contains(Coord))
			{
				Color = FColor::Black;
			}
			else if (State.ReachableTiles.Contains(Coord))
			{
				Color = FColor::Cyan;
			}

			DrawDebugBox(GetWorld(), CoordToWorld(X, Y, 0.0f), TileExtent, Color, false, DrawDuration, 0, 2.0f);
		}
	}

	for (const FJGBattleUnitState& Unit : State.Units)
	{
		if (!Unit.IsAlive())
		{
			continue;
		}

		FColor UnitColor = Unit.Team == EJGBattleTeam::Player ? FColor::Green : FColor::Red;
		if (State.SelectedPlayerUnit == Unit.Handle)
		{
			UnitColor = FColor::Yellow;
		}

		DrawDebugBox(GetWorld(), CoordToWorld(Unit.Pos.X, Unit.Pos.Y, UnitDrawHeight), UnitExtent, UnitColor, false, DrawDuration, 0, 6.0f);
	}

	return true;
}

FVector AJGBattleDebugBoardActor::CoordToWorld(const int32 X, const int32 Y, const float Height) const
{
	const FVector Origin = GetActorLocation();
	return Origin + FVector(X * TileSizeUU, Y * TileSizeUU, Height);
}

UJGBattleSubsystem* AJGBattleDebugBoardActor::ResolveBattleSubsystem() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UJGBattleSubsystem>();
	}
	return nullptr;
}
