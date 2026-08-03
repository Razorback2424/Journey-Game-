#include "Overworld/JGGridBlueprintLibrary.h"

namespace JGGridInternal
{
	static float SanitizeTileSize(const float TileSizeUU)
	{
		return FMath::Abs(TileSizeUU) > KINDA_SMALL_NUMBER ? FMath::Abs(TileSizeUU) : 32.0f;
	}
}

FVector UJGGridBlueprintLibrary::SnapWorldLocationToGrid(
	const FVector WorldLocation,
	const float TileSizeUU,
	const bool bPreserveInputZ,
	const float OverrideZ)
{
	const float SafeTile = JGGridInternal::SanitizeTileSize(TileSizeUU);
	const float SnappedX = FMath::RoundToFloat(WorldLocation.X / SafeTile) * SafeTile;
	const float SnappedY = FMath::RoundToFloat(WorldLocation.Y / SafeTile) * SafeTile;
	const float Z = bPreserveInputZ ? WorldLocation.Z : OverrideZ;
	return FVector(SnappedX, SnappedY, Z);
}

FJGGridCoord UJGGridBlueprintLibrary::GridCoordFromWorld(const FVector WorldLocation, const float TileSizeUU)
{
	const float SafeTile = JGGridInternal::SanitizeTileSize(TileSizeUU);
	return FJGGridCoord(
		FMath::RoundToInt(WorldLocation.X / SafeTile),
		FMath::RoundToInt(WorldLocation.Y / SafeTile));
}

FVector UJGGridBlueprintLibrary::WorldFromGridCoord(const FJGGridCoord GridCoord, const float TileSizeUU, const float Z)
{
	const float SafeTile = JGGridInternal::SanitizeTileSize(TileSizeUU);
	return FVector(static_cast<float>(GridCoord.X) * SafeTile, static_cast<float>(GridCoord.Y) * SafeTile, Z);
}

FVector2D UJGGridBlueprintLibrary::DirectionToGridStep(const FVector2D InputDirection, const bool bPreferXOnTie)
{
	if (InputDirection.IsNearlyZero())
	{
		return FVector2D::ZeroVector;
	}

	const float AbsX = FMath::Abs(InputDirection.X);
	const float AbsY = FMath::Abs(InputDirection.Y);

	if (AbsX > AbsY || (bPreferXOnTie && FMath::IsNearlyEqual(AbsX, AbsY)))
	{
		return FVector2D(FMath::Sign(InputDirection.X), 0.0f);
	}

	return FVector2D(0.0f, FMath::Sign(InputDirection.Y));
}

bool UJGGridBlueprintLibrary::IsWorldLocationOnGrid(const FVector WorldLocation, const float TileSizeUU, const float Tolerance)
{
	const FVector Snapped = SnapWorldLocationToGrid(WorldLocation, TileSizeUU, true, 0.0f);
	return FMath::IsNearlyEqual(WorldLocation.X, Snapped.X, Tolerance)
		&& FMath::IsNearlyEqual(WorldLocation.Y, Snapped.Y, Tolerance);
}

FVector UJGGridBlueprintLibrary::GridStepToWorldDelta(const FVector2D GridStepDirection, const float TileSizeUU)
{
	const FVector2D Step = DirectionToGridStep(GridStepDirection);
	const float SafeTile = JGGridInternal::SanitizeTileSize(TileSizeUU);
	return FVector(Step.X * SafeTile, Step.Y * SafeTile, 0.0f);
}

FVector UJGGridBlueprintLibrary::ForwardVectorFromGridStep(const FVector2D GridStepDirection)
{
	const FVector2D Step = DirectionToGridStep(GridStepDirection);
	return FVector(Step.X, Step.Y, 0.0f);
}
