#include "Overworld/JGOverworldPlayerControllerBase.h"

#include "Core/JGLog.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "Overworld/JGOverworldCharacterBase.h"
#include "Overworld/JGOverworldInteractionComponent.h"
#include "UObject/ConstructorHelpers.h"

AJGOverworldPlayerControllerBase::AJGOverworldPlayerControllerBase()
{
	bShowMouseCursor = false;

	if (static ConstructorHelpers::FObjectFinder<UInputMappingContext> MappingContext(TEXT("/Game/Overworld/Input/IMC_Overworld.IMC_Overworld")); MappingContext.Succeeded())
	{
		DefaultInputMappingContext = MappingContext.Object;
	}

	if (static ConstructorHelpers::FObjectFinder<UInputAction> MoveUpAction(TEXT("/Game/Overworld/Input/IA_MoveUp.IA_MoveUp")); MoveUpAction.Succeeded())
	{
		MoveUpInputAction = MoveUpAction.Object;
	}

	if (static ConstructorHelpers::FObjectFinder<UInputAction> MoveDownAction(TEXT("/Game/Overworld/Input/IA_MoveDown.IA_MoveDown")); MoveDownAction.Succeeded())
	{
		MoveDownInputAction = MoveDownAction.Object;
	}

	if (static ConstructorHelpers::FObjectFinder<UInputAction> MoveLeftAction(TEXT("/Game/Overworld/Input/IA_MoveLeft.IA_MoveLeft")); MoveLeftAction.Succeeded())
	{
		MoveLeftInputAction = MoveLeftAction.Object;
	}

	if (static ConstructorHelpers::FObjectFinder<UInputAction> MoveRightAction(TEXT("/Game/Overworld/Input/IA_MoveRight.IA_MoveRight")); MoveRightAction.Succeeded())
	{
		MoveRightInputAction = MoveRightAction.Object;
	}
}

void AJGOverworldPlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (DefaultInputMappingContext)
			{
				InputSubsystem->AddMappingContext(DefaultInputMappingContext, DefaultInputMappingPriority);
			}
		}
	}

	BindInteractionComponentDelegates(GetPawn());

	UE_LOG(LogJourneyOverworld, Verbose, TEXT("Overworld PlayerController initialized: %s"), *GetName());
}

void AJGOverworldPlayerControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	BindInteractionComponentDelegates(InPawn);
}

void AJGOverworldPlayerControllerBase::OnUnPossess()
{
	if (BoundInteractionComponent.IsValid())
	{
		BoundInteractionComponent->OnFocusedInteractableChanged.RemoveAll(this);
		BoundInteractionComponent->OnInteractionAttempted.RemoveAll(this);
		BoundInteractionComponent.Reset();
	}

	Super::OnUnPossess();
}

void AJGOverworldPlayerControllerBase::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (MoveUpInputAction)
		{
			EnhancedInputComponent->BindAction(MoveUpInputAction, ETriggerEvent::Started, this, &AJGOverworldPlayerControllerBase::HandleMoveUpInput);
		}
		if (MoveDownInputAction)
		{
			EnhancedInputComponent->BindAction(MoveDownInputAction, ETriggerEvent::Started, this, &AJGOverworldPlayerControllerBase::HandleMoveDownInput);
		}
		if (MoveLeftInputAction)
		{
			EnhancedInputComponent->BindAction(MoveLeftInputAction, ETriggerEvent::Started, this, &AJGOverworldPlayerControllerBase::HandleMoveLeftInput);
		}
		if (MoveRightInputAction)
		{
			EnhancedInputComponent->BindAction(MoveRightInputAction, ETriggerEvent::Started, this, &AJGOverworldPlayerControllerBase::HandleMoveRightInput);
		}
		if (InteractInputAction)
		{
			EnhancedInputComponent->BindAction(InteractInputAction, ETriggerEvent::Started, this, &AJGOverworldPlayerControllerBase::HandleInteractInput);
		}
	}

	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::W, IE_Pressed, this, &AJGOverworldPlayerControllerBase::HandleMoveUpInput);
		InputComponent->BindKey(EKeys::Up, IE_Pressed, this, &AJGOverworldPlayerControllerBase::HandleMoveUpInput);
		InputComponent->BindKey(EKeys::S, IE_Pressed, this, &AJGOverworldPlayerControllerBase::HandleMoveDownInput);
		InputComponent->BindKey(EKeys::Down, IE_Pressed, this, &AJGOverworldPlayerControllerBase::HandleMoveDownInput);
		InputComponent->BindKey(EKeys::A, IE_Pressed, this, &AJGOverworldPlayerControllerBase::HandleMoveLeftInput);
		InputComponent->BindKey(EKeys::Left, IE_Pressed, this, &AJGOverworldPlayerControllerBase::HandleMoveLeftInput);
		InputComponent->BindKey(EKeys::D, IE_Pressed, this, &AJGOverworldPlayerControllerBase::HandleMoveRightInput);
		InputComponent->BindKey(EKeys::Right, IE_Pressed, this, &AJGOverworldPlayerControllerBase::HandleMoveRightInput);
		InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AJGOverworldPlayerControllerBase::HandleInteractInput);
		InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &AJGOverworldPlayerControllerBase::HandleInteractInput);
	}
}

bool AJGOverworldPlayerControllerBase::TryInteractFocused()
{
	if (AJGOverworldCharacterBase* OverworldCharacter = Cast<AJGOverworldCharacterBase>(GetPawn()))
	{
		if (UJGOverworldInteractionComponent* InteractionComponent = OverworldCharacter->GetInteractionComponent())
		{
			return InteractionComponent->TryInteractFocused();
		}
	}

	return false;
}

void AJGOverworldPlayerControllerBase::BindInteractionComponentDelegates(APawn* InPawn)
{
	if (BoundInteractionComponent.IsValid())
	{
		BoundInteractionComponent->OnFocusedInteractableChanged.RemoveAll(this);
		BoundInteractionComponent->OnInteractionAttempted.RemoveAll(this);
		BoundInteractionComponent.Reset();
	}

	if (AJGOverworldCharacterBase* OverworldCharacter = Cast<AJGOverworldCharacterBase>(InPawn))
	{
		if (UJGOverworldInteractionComponent* InteractionComponent = OverworldCharacter->GetInteractionComponent())
		{
			InteractionComponent->OnFocusedInteractableChanged.AddUniqueDynamic(this, &AJGOverworldPlayerControllerBase::HandleFocusedInteractableChanged);
			InteractionComponent->OnInteractionAttempted.AddUniqueDynamic(this, &AJGOverworldPlayerControllerBase::HandleInteractionAttempted);
			BoundInteractionComponent = InteractionComponent;
		}
	}
}

void AJGOverworldPlayerControllerBase::HandleFocusedInteractableChanged(AActor* PreviousInteractable, AActor* NewInteractable)
{
	BP_OnFocusedInteractableChanged(PreviousInteractable, NewInteractable);
}

void AJGOverworldPlayerControllerBase::HandleInteractionAttempted(AActor* TargetActor, const bool bSuccess)
{
	BP_OnInteractionAttempted(TargetActor, bSuccess);
}

void AJGOverworldPlayerControllerBase::HandleInteractInput()
{
	TryInteractFocused();
}

void AJGOverworldPlayerControllerBase::HandleMoveUpInput()
{
	if (AJGOverworldCharacterBase* OverworldCharacter = Cast<AJGOverworldCharacterBase>(GetPawn()))
	{
		OverworldCharacter->RequestMoveUp();
	}
}

void AJGOverworldPlayerControllerBase::HandleMoveDownInput()
{
	if (AJGOverworldCharacterBase* OverworldCharacter = Cast<AJGOverworldCharacterBase>(GetPawn()))
	{
		OverworldCharacter->RequestMoveDown();
	}
}

void AJGOverworldPlayerControllerBase::HandleMoveLeftInput()
{
	if (AJGOverworldCharacterBase* OverworldCharacter = Cast<AJGOverworldCharacterBase>(GetPawn()))
	{
		OverworldCharacter->RequestMoveLeft();
	}
}

void AJGOverworldPlayerControllerBase::HandleMoveRightInput()
{
	if (AJGOverworldCharacterBase* OverworldCharacter = Cast<AJGOverworldCharacterBase>(GetPawn()))
	{
		OverworldCharacter->RequestMoveRight();
	}
}
