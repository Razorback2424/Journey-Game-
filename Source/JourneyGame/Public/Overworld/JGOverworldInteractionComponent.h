#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "JGOverworldInteractionComponent.generated.h"

class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FJGFocusedInteractableChangedSignature, AActor*, PreviousInteractable, AActor*, NewInteractable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FJGInteractionAttemptedSignature, AActor*, TargetActor, bool, bSuccess);

UCLASS(ClassGroup = (JourneyGame), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class JOURNEYGAME_API UJGOverworldInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UJGOverworldInteractionComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bInteractionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bAutoRefreshFocus = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bDrawDebugTrace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = "1.0"))
	float InteractionTraceDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TEnumAsByte<ECollisionChannel> InteractionTraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText DefaultPromptText;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<AActor> FocusedInteractableActor = nullptr;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FJGFocusedInteractableChangedSignature OnFocusedInteractableChanged;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FJGInteractionAttemptedSignature OnInteractionAttempted;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	AActor* RefreshInteractableFocus();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool UpdateFocusedInteractable(AActor* CandidateInteractable);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool TryInteractFocused();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool TryInteractActor(AActor* TargetActor);

	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetFocusedInteractable() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool HasFocusedInteractable() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	FText GetFocusedPromptText() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* ResolveInteractionSourceActor() const;

protected:
	void SetFocusedInteractable(AActor* NewFocusedInteractable);
};
