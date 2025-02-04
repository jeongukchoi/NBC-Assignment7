
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

class UInputAction;

UCLASS()
class PAWNCHARACTER_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMyPlayerController();

	UPROPERTY(EditAnywhere)
	class UInputMappingContext* IMC_Ground;

	UPROPERTY(EditAnywhere)
	class UInputMappingContext* IMC_Flying;

	// Ground Control
	UPROPERTY(EditAnywhere)
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere)
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere)
	UInputAction* MoveUpAction;

	// Air Control
	UPROPERTY(EditAnywhere)
	UInputAction* RollAction;
	UPROPERTY(EditAnywhere)
	UInputAction* PitchAction;
	UPROPERTY(EditAnywhere)
	UInputAction* YawAction;
	UPROPERTY(EditAnywhere)
	UInputAction* AccelerateAction;
	

protected:
	
	virtual void BeginPlay() override;
};
