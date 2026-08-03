#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Overworld/JGOverworldTypes.h"
#include "JGMapTransitionTriggerBase.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;
struct FHitResult;

UCLASS(Blueprintable)
class JOURNEYGAME_API AJGMapTransitionTriggerBase : public AActor
{
	GENERATED_BODY()

public:
	AJGMapTransitionTriggerBase();

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> Trigger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	FName DestinationMapId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	FName DestinationSpawnTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	bool bSeamlessTravel = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	bool bOneShot = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	bool bDebugLog = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	FString DebugReason;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Transition")
	bool bConsumed = false;

	UFUNCTION(BlueprintCallable, Category = "Transition")
	bool CanTriggerTransition(AActor* TriggeringActor) const;

	UFUNCTION(BlueprintCallable, Category = "Transition")
	bool TryRequestTransition(AActor* TriggeringActor);

	UFUNCTION(BlueprintPure, Category = "Transition")
	FJGMapTransitionRequest BuildTransitionRequest() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Transition", meta = (DisplayName = "On Transition Requested"))
	void BP_OnTransitionRequested(AActor* TriggeringActor);

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
