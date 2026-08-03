#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Encounters/JGEncounterSourceInterface.h"
#include "JGEncounterZoneBase.generated.h"

class UBoxComponent;
class UJGEncounterTableDataAsset;
class UPrimitiveComponent;
class USceneComponent;
struct FHitResult;

UCLASS(Blueprintable)
class JOURNEYGAME_API AJGEncounterZoneBase : public AActor, public IJGEncounterSourceInterface
{
	GENERATED_BODY()

public:
	AJGEncounterZoneBase();

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> Trigger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter")
	TObjectPtr<UJGEncounterTableDataAsset> EncounterTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter")
	bool bOneShot = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter")
	bool bDebugLog = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Encounter")
	bool bConsumed = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "Encounter", meta = (DisplayName = "On Encounter Triggered"))
	void BP_OnEncounterTriggered(AActor* TriggeringActor);

	virtual UJGEncounterTableDataAsset* GetEncounterTable_Implementation() const override;
	virtual bool CanTriggerEncounter_Implementation(AActor* TriggeringActor) const override;
	virtual void HandleEncounterConsumed_Implementation(AActor* TriggeringActor) override;

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
