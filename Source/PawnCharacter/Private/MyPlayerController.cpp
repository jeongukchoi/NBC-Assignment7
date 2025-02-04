

#include "MyPlayerController.h"
#include "EnhancedInputSubsystems.h"

AMyPlayerController::AMyPlayerController() :
	IMC_Ground(nullptr),
	IMC_Flying(nullptr),
	LookAction(nullptr),
	MoveAction(nullptr),
	MoveUpAction(nullptr),
	RollAction(nullptr),
	PitchAction(nullptr),
	YawAction(nullptr),
	AccelerateAction(nullptr)
{
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (IMC_Ground)
			{
				Subsystem->AddMappingContext(IMC_Ground, 0);
			}
		}
	}
}