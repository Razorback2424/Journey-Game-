#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "JGOverworldPlayerControllerBase.generated.h"

class UInputAction;
class UInputMappingContext;
class UJGOverworldInteractionComponent;
class AActor;
class APawn;

UCLASS(Blueprintable)
class JOURNEYGAME_API AJGOverworldPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	AJGOverworldPlayerControllerBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bEnableOverworldDebugInput = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (ClampMin = "0"))
	int32 DefaultInputMappingPriority = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractInputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveUpInputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveDownInputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveLeftInputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveRightInputAction = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Overworld|Interaction")
	bool TryInteractFocused();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Overworld|Interaction", meta = (DisplayName = "On Focused Interactable Changed"))
	void BP_OnFocusedInteractableChanged(AActor* PreviousInteractable, AActor* NewInteractable);

	UFUNCTION(BlueprintImplementableEvent, Category = "Overworld|Interaction", meta = (DisplayName = "On Interaction Attempted"))
	void BP_OnInteractionAttempted(AActor* TargetActor, bool bSuccess);

private:
	TWeakObjectPtr<UJGOverworldInteractionComponent> BoundInteractionComponent;

	void BindInteractionComponentDelegates(APawn* InPawn);

	UFUNCTION()
	void HandleFocusedInteractableChanged(AActor* PreviousInteractable, AActor* NewInteractable);

	UFUNCTION()
	void HandleInteractionAttempted(AActor* TargetActor, bool bSuccess);

	void HandleInteractInput();
	void HandleMoveUpInput();
	void HandleMoveDownInput();
	void HandleMoveLeftInput();
	void HandleMoveRightInput();
};
