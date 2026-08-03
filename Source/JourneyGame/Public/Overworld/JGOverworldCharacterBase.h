#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Data/JGGridTypes.h"
#include "JGOverworldCharacterBase.generated.h"

class UCameraComponent;
class UJGOverworldInteractionComponent;
class UPaperSpriteComponent;
class USpringArmComponent;
class UWorld;

UCLASS(Blueprintable)
class JOURNEYGAME_API AJGOverworldCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AJGOverworldCharacterBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> OverworldCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> SpriteVisual;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UJGOverworldInteractionComponent> InteractionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overworld|Grid")
	FJGGridSettings GridSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overworld|Movement")
	bool bMovementEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overworld|Movement")
	bool bPreferXOnDirectionalTie = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Overworld|Movement")
	bool bStepMovementInProgress = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Overworld|Movement")
	FVector2D CurrentMoveDirection = FVector2D::ZeroVector;

	UFUNCTION(BlueprintCallable, Category = "Overworld|Grid")
	void SnapActorToGrid(bool bPreserveZ = true);

	UFUNCTION(BlueprintPure, Category = "Overworld|Interaction")
	UJGOverworldInteractionComponent* GetInteractionComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Overworld|Movement")
	bool RequestMoveInDirection(FVector2D InputDirection);

	UFUNCTION(BlueprintCallable, Category = "Overworld|Movement")
	bool RequestMoveUp();

	UFUNCTION(BlueprintCallable, Category = "Overworld|Movement")
	bool RequestMoveDown();

	UFUNCTION(BlueprintCallable, Category = "Overworld|Movement")
	bool RequestMoveLeft();

	UFUNCTION(BlueprintCallable, Category = "Overworld|Movement")
	bool RequestMoveRight();

	UFUNCTION(BlueprintPure, Category = "Overworld|Movement")
	bool IsStepMovementInProgress() const;

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Overworld|Movement", meta = (DisplayName = "On Step Move Started"))
	void BP_OnStepMoveStarted(FVector2D StepDirection, FVector StartLocation, FVector TargetLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Overworld|Movement", meta = (DisplayName = "On Step Move Finished"))
	void BP_OnStepMoveFinished(FVector2D StepDirection, FVector FinalLocation);

private:
	FVector StepStartLocation = FVector::ZeroVector;
	FVector StepTargetLocation = FVector::ZeroVector;
	float StepElapsedTime = 0.0f;

	bool CanStepToLocation(const FVector& StartLocation, const FVector& TargetLocation) const;
	void FinishStepMove();
};
