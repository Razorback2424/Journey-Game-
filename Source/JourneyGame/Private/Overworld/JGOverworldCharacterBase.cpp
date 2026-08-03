#include "Overworld/JGOverworldCharacterBase.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Core/JGDeveloperSettings.h"
#include "Core/JGLog.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Overworld/JGGridBlueprintLibrary.h"
#include "Overworld/JGOverworldInteractionComponent.h"
#include "PaperSpriteComponent.h"
#include "PaperSprite.h"
#include "UObject/ConstructorHelpers.h"
#include "WorldCollision.h"

AJGOverworldCharacterBase::AJGOverworldCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	const UJGDeveloperSettings* Settings = UJGDeveloperSettings::Get();
	if (Settings)
	{
		GridSettings = Settings->DefaultGridSettings;
	}

	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f));

	OverworldCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("OverworldCamera"));
	OverworldCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	OverworldCamera->ProjectionMode = ECameraProjectionMode::Orthographic;
	OverworldCamera->OrthoWidth = 704.0f;

	SpriteVisual = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteVisual"));
	SpriteVisual->SetupAttachment(GetRootComponent());
	SpriteVisual->SetRelativeLocation(FVector(0.0f, 0.0f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
	if (static ConstructorHelpers::FObjectFinder<UPaperSprite> DefaultSprite(TEXT("/Game/Art/Characters/KenneyRoguelike/roguelikeChar_transparent_Sprite.roguelikeChar_transparent_Sprite")); DefaultSprite.Succeeded())
	{
		SpriteVisual->SetSprite(DefaultSprite.Object);
	}

	InteractionComponent = CreateDefaultSubobject<UJGOverworldInteractionComponent>(TEXT("InteractionComponent"));
}

void AJGOverworldCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (GridSettings.bSnapOnBeginPlay)
	{
		SnapActorToGrid(true);
	}

	SetActorTickEnabled(false);

	UE_LOG(LogJourneyOverworld, Verbose, TEXT("Overworld character base begin play: %s"), *GetName());
}

void AJGOverworldCharacterBase::SnapActorToGrid(const bool bPreserveZ)
{
	const FVector Snapped = UJGGridBlueprintLibrary::SnapWorldLocationToGrid(GetActorLocation(), GridSettings.TileSizeUU, bPreserveZ, GetActorLocation().Z);
	SetActorLocation(Snapped);
}

UJGOverworldInteractionComponent* AJGOverworldCharacterBase::GetInteractionComponent() const
{
	return InteractionComponent;
}

bool AJGOverworldCharacterBase::RequestMoveInDirection(const FVector2D InputDirection)
{
	if (!bMovementEnabled || bStepMovementInProgress)
	{
		return false;
	}

	const FVector2D StepDirection = UJGGridBlueprintLibrary::DirectionToGridStep(InputDirection, bPreferXOnDirectionalTie);
	if (StepDirection.IsNearlyZero())
	{
		return false;
	}

	SnapActorToGrid(true);

	const FVector StartLocation = GetActorLocation();
	const FVector TargetLocation = StartLocation + UJGGridBlueprintLibrary::GridStepToWorldDelta(StepDirection, GridSettings.TileSizeUU);
	if (!CanStepToLocation(StartLocation, TargetLocation))
	{
		UE_LOG(LogJourneyOverworld, Verbose, TEXT("Step move blocked for %s"), *GetName());
		return false;
	}

	CurrentMoveDirection = StepDirection;
	StepStartLocation = StartLocation;
	StepTargetLocation = TargetLocation;
	StepElapsedTime = 0.0f;
	bStepMovementInProgress = true;
	SetActorRotation(UJGGridBlueprintLibrary::ForwardVectorFromGridStep(StepDirection).Rotation());
	SetActorTickEnabled(true);

	BP_OnStepMoveStarted(CurrentMoveDirection, StepStartLocation, StepTargetLocation);

	if (GridSettings.StepDuration <= KINDA_SMALL_NUMBER)
	{
		FinishStepMove();
	}

	return true;
}

bool AJGOverworldCharacterBase::RequestMoveUp()
{
	return RequestMoveInDirection(FVector2D(0.0f, 1.0f));
}

bool AJGOverworldCharacterBase::RequestMoveDown()
{
	return RequestMoveInDirection(FVector2D(0.0f, -1.0f));
}

bool AJGOverworldCharacterBase::RequestMoveLeft()
{
	return RequestMoveInDirection(FVector2D(-1.0f, 0.0f));
}

bool AJGOverworldCharacterBase::RequestMoveRight()
{
	return RequestMoveInDirection(FVector2D(1.0f, 0.0f));
}

bool AJGOverworldCharacterBase::IsStepMovementInProgress() const
{
	return bStepMovementInProgress;
}

void AJGOverworldCharacterBase::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bStepMovementInProgress)
	{
		SetActorTickEnabled(false);
		return;
	}

	StepElapsedTime += FMath::Max(0.0f, DeltaTime);
	const float SafeDuration = FMath::Max(GridSettings.StepDuration, KINDA_SMALL_NUMBER);
	const float Alpha = FMath::Clamp(StepElapsedTime / SafeDuration, 0.0f, 1.0f);
	const FVector NewLocation = FMath::Lerp(StepStartLocation, StepTargetLocation, Alpha);
	SetActorLocation(NewLocation, false);

	if (Alpha >= 1.0f)
	{
		FinishStepMove();
	}
}

bool AJGOverworldCharacterBase::CanStepToLocation(const FVector& StartLocation, const FVector& TargetLocation) const
{
	const UCapsuleComponent* CapsuleComponent = GetCapsuleComponent();
	if (!CapsuleComponent || !GetWorld())
	{
		return true;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(JGOverworldStepMove), false, this);
	FCollisionResponseParams ResponseParams;
	CapsuleComponent->InitSweepCollisionParams(QueryParams, ResponseParams);

	const FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(
		CapsuleComponent->GetScaledCapsuleRadius(),
		CapsuleComponent->GetScaledCapsuleHalfHeight());

	FHitResult Hit;
	const bool bBlocked = GetWorld()->SweepSingleByChannel(
		Hit,
		StartLocation,
		TargetLocation,
		FQuat::Identity,
		CapsuleComponent->GetCollisionObjectType(),
		CollisionShape,
		QueryParams,
		ResponseParams);

	return !bBlocked;
}

void AJGOverworldCharacterBase::FinishStepMove()
{
	SetActorLocation(UJGGridBlueprintLibrary::SnapWorldLocationToGrid(StepTargetLocation, GridSettings.TileSizeUU, true, StepTargetLocation.Z), false);
	bStepMovementInProgress = false;
	StepElapsedTime = 0.0f;
	SetActorTickEnabled(false);

	if (InteractionComponent)
	{
		InteractionComponent->RefreshInteractableFocus();
	}

	BP_OnStepMoveFinished(CurrentMoveDirection, GetActorLocation());
}
