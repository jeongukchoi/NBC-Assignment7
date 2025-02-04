
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyPawn.generated.h"

struct FInputActionValue;

UCLASS()
class PAWNCHARACTER_API AMyPawn : public APawn
{
	GENERATED_BODY()

public:
	AMyPawn();

protected:
	virtual void BeginPlay() override;

	// CAMERA - Pitch/Yaw with mouse position
	UFUNCTION()
	void Look(const FInputActionValue& Value);

	// GROUND - XY movement with W/A/S/D keys
	UFUNCTION()
	void MoveGround(const FInputActionValue& Value);

	// GROUND - Take off with Space Bar (followed by W right after to continue accelerating upwards)
	UFUNCTION()
	void MoveUp(const FInputActionValue& Value);

	// AIR - Roll with numpad 4/6
	UFUNCTION()
	void Roll(const FInputActionValue& Value);

	// AIR - Reset roll in Tick when Roll trigger is completed
	UFUNCTION()
	void ResetRoll(const FInputActionValue& Value);

	// AIR - Pitch control with numpad 5/8
	UFUNCTION()
	void Pitch(const FInputActionValue& Value);

	// AIR - Reset pitch in Tick when Pitch trigger is completed
	UFUNCTION()
	void ResetPitch(const FInputActionValue& Value);

	// AIR - Yaw with A/D keys
	UFUNCTION()
	void Yaw(const FInputActionValue& Value);

	// AIR - Acceleration upwards (determines orientation of movement when added with gravity vector)
	UFUNCTION()
	void Accelerate(const FInputActionValue& Value);

	// AIR - Turn on free fall in Tick
	UFUNCTION()
	void StopAccelerate(const FInputActionValue& Value);

	// Line trace to detect floor
	UFUNCTION()
	void LineTraceFloor();

	// Switches IMC (Ground <-> Flying)
	UFUNCTION()
	void SwitchIMC(bool bShouldFly);

	UPROPERTY(EditAnywhere, Category = "Properties|Camera")
	float LookPitchMax;

	UPROPERTY(EditAnywhere, Category = "Properties|Movement")
	float GroundSpeed;

	UPROPERTY(EditAnywhere, Category = "Properties|Movement")
	float AccelerateForce;

	UPROPERTY(EditAnywhere, Category = "Properties|Movement")
	float AirPitchSpeed;

	UPROPERTY(EditAnywhere, Category = "Properties|Movement")
	float AirRollSpeed;

	UPROPERTY(EditAnywhere, Category = "Properties|Movement")
	float AirYawSpeed;

	UPROPERTY(EditAnywhere, Category = "Properties|Movement")
	float AirPitchMax;

	UPROPERTY(EditAnywhere, Category = "Properties|Movement")
	float AirRollMax;

	UPROPERTY(VisibleAnywhere, Category = "Properties|Movement")
	bool bShouldResetRoll;

	UPROPERTY(EditAnywhere, Category = "Properties|Movement")
	float ResetRollDuration;
	float ResetRollAlpha;
	float ResetRollStart;

	UPROPERTY(VisibleAnywhere, Category = "Properties|Movement")
	bool bShouldResetPitch;

	UPROPERTY(EditAnywhere, Category = "Properties|Movement")
	float ResetPitchDuration;
	float ResetPitchAlpha;
	float ResetPitchStart;

	UPROPERTY(EditAnywhere, Category = "Properties|Gravity")
	float GravitationalAcceleration;

	UPROPERTY(EditAnywhere, Category = "Properties|Gravity")
	float FreeFallSpeed;

	UPROPERTY(VisibleAnywhere, Category = "Properties|Gravity")
	bool bShouldActivateFallTime;
	float FallTimeElapsed;

	UPROPERTY(EditAnywhere, Category = "Properties|Collision")
	TEnumAsByte<ECollisionChannel> TraceChannel;

	UPROPERTY(VisibleAnywhere, Category = "Properties|Collision")
	bool bIsInTheAir;


public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Properties|Components")
	class UBoxComponent* BoxComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties|Components")
	USkeletalMeshComponent* SkeletalMeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties|Components")
	class USpringArmComponent* SpringArmComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties|Components")
	class UCameraComponent* CameraComp;
};
