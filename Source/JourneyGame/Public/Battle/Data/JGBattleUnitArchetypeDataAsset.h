#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Battle/JGBattleTypes.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif
#include "JGBattleUnitArchetypeDataAsset.generated.h"

UCLASS(BlueprintType)
class JOURNEYGAME_API UJGBattleUnitArchetypeDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	FName ArchetypeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	EJGBattleUnitRole Role = EJGBattleUnitRole::Brute;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle", meta = (ClampMin = "1"))
	int32 MaxHp = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle", meta = (ClampMin = "1"))
	int32 AC = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	int32 AttackBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	int32 InitiativeBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	FString DamageDice = TEXT("1d6");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle", meta = (ClampMin = "1"))
	int32 MoveRange = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle", meta = (ClampMin = "1"))
	int32 AttackRange = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	EJGBattleBonusAbilityType BonusAbilityType = EJGBattleBonusAbilityType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	FText BonusAbilityLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	FGameplayTagContainer Tags;

	UFUNCTION(BlueprintCallable, Category = "Battle|Validation")
	bool ValidateArchetype(FString& OutError) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};

