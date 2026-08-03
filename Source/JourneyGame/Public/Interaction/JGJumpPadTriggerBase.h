#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JGJumpPadTriggerBase.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;
struct FHitResult;

UCLASS(Blueprintable)
class JOURNEYGAME_API AJGJumpPadTriggerBase : public AActor
{
	GENERATED_BODY()

public:
	AJGJumpPadTriggerBase();

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> Trigger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "JumpPad")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "JumpPad")
	bool bOneShot = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "JumpPad")
	bool bDebugLog = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "JumpPad")
	FVector LaunchVelocity = FVector(0.0f, 0.0f, 900.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "JumpPad")
	bool bOverrideXY = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "JumpPad")
	bool bOverrideZ = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "JumpPad")
	bool bConsumed = false;

	UFUNCTION(BlueprintCallable, Category = "JumpPad")
	bool CanLaunchActor(AActor* OtherActor) const;

	UFUNCTION(BlueprintCallable, Category = "JumpPad")
	bool TryLaunchActor(AActor* OtherActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "JumpPad", meta = (DisplayName = "On Actor Launched"))
	void BP_OnActorLaunched(AActor* OtherActor);

protected:
	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
