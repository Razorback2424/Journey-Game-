#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JGBattleDebugBoardActor.generated.h"

class UJGBattleSubsystem;

UCLASS(Blueprintable)
class JOURNEYGAME_API AJGBattleDebugBoardActor : public AActor
{
	GENERATED_BODY()

public:
	AJGBattleDebugBoardActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle|Debug")
	float TileSizeUU = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle|Debug")
	float TileDrawHeight = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle|Debug")
	float UnitDrawHeight = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle|Debug")
	float DrawDuration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle|Debug")
	bool bRefreshOnTick = true;

	UFUNCTION(BlueprintCallable, Category = "Battle|Debug")
	bool RefreshFromBattleSubsystem();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	FVector CoordToWorld(int32 X, int32 Y, float Height) const;
	UJGBattleSubsystem* ResolveBattleSubsystem() const;
};
