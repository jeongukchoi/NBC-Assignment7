

#include "MyPawn.h"
#include "Components/BoxComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "MyPlayerController.h"

AMyPawn::AMyPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// set capsule component as root component
	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	SetRootComponent(BoxComp);
	BoxComp->SetSimulatePhysics(false);

	// skeletal mesh component
	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	SkeletalMeshComp->SetupAttachment(RootComponent);
	SkeletalMeshComp->SetSimulatePhysics(false);
	ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshAsset(TEXT("/Game/StylizedCreaturesBundle/Meshes/Eagle/SK_Eagle1.SK_Eagle1"));
	if (SkeletalMeshAsset.Succeeded())
	{
		SkeletalMeshComp->SetSkeletalMesh(SkeletalMeshAsset.Object);
	}

	// spring arm component
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 350.f;
	SpringArmComp->SocketOffset = FVector(0, 0, 100.f);
	SpringArmComp->bUsePawnControlRotation = false;

	// camera component
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	// Limits
	LookPitchMax = 75.f;
	AirPitchMax = 30.f;
	AirRollMax = 30.f;

	// Speed
	GroundSpeed = 12.f;
	AccelerateForce = 11.f;
	AirPitchSpeed = 0.3f;
	AirRollSpeed = 0.3f;
	AirYawSpeed = 0.5f;

	// Gravity
	GravitationalAcceleration = 9.8f;
	FallTimeElapsed = 0.f;
	FreeFallSpeed = 0.5f;
	bShouldActivateFallTime = false;

	// Collision
	TraceChannel = ECC_Visibility;
	bIsInTheAir = false;

	// Smooth reset pitch/roll to zero if there is no pitch/roll input
	bShouldResetPitch = false;
	bShouldResetRoll = false;
	ResetRollStart = 0.f;
	ResetRollAlpha = 0.f;
	ResetRollDuration = 1.5f;
	ResetPitchStart = 0.f;
	ResetPitchAlpha = 0.f;
	ResetPitchDuration = 1.5f;
}

void AMyPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AMyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update air/ground condition & reset values/flags according to condition change
	LineTraceFloor();

	// When in the air
	if (bIsInTheAir)
	{
		// Downward (i.e. negative Z) acceleration due to gravity
		if (bShouldActivateFallTime)
		{
			// a. When not accelerating (i.e. free fall), grdually accelerate over fall time
			AddActorWorldOffset(FVector(0, 0, -1 * GravitationalAcceleration * FallTimeElapsed * FreeFallSpeed), true);
			FallTimeElapsed += DeltaTime;
		}
		else
		{
			// b. When accelerating, apply constant gravity to determine final orientation by addition with the acceleration vector
			AddActorWorldOffset(FVector(0, 0, -1 * GravitationalAcceleration), true);
		}

		// Reset roll when there is no player input
		if (bShouldResetRoll)
		{
			// Smooth reset roll using linear interpolation over reset duration
			FRotator MeshRotation = SkeletalMeshComp->GetRelativeRotation();
			SkeletalMeshComp->SetRelativeRotation(FRotator(MeshRotation.Pitch, MeshRotation.Yaw, FMath::Lerp(ResetRollStart, 0, ResetRollAlpha)));
			ResetRollAlpha += DeltaTime / FMath::Max(ResetRollDuration, 1.f);
			
			if (ResetRollAlpha >= 0.999f)
			{
				ResetRollAlpha = 0.f;
				ResetRollStart = 0.f;
				bShouldResetRoll = false;
			}
		}

		// Reset pitch when there is no player input
		if (bShouldResetPitch)
		{
			// Smooth reset pitch using linear interpolation over reset duration
			FRotator MeshRotation = SkeletalMeshComp->GetRelativeRotation();
			SkeletalMeshComp->SetRelativeRotation(FRotator(FMath::Lerp(ResetPitchStart, 0, ResetPitchAlpha), MeshRotation.Yaw, MeshRotation.Roll));
			ResetPitchAlpha += DeltaTime / FMath::Max(ResetPitchDuration, 1.f);
			
			if (ResetPitchAlpha >= 0.999f)
			{
				ResetPitchAlpha = 0.f;
				ResetPitchStart = 0.f;
				bShouldResetPitch = false;
			}
		}
	}
}

// Control camera with mouse movement
void AMyPawn::Look(const FInputActionValue& Value)
{
	if (!Controller) return;

	FVector2D LookInput = Value.Get<FVector2D>();
	if (!FMath::IsNearlyZero(LookInput.Y))
	{
		// Limit pitch of camera
		if (abs(SpringArmComp->GetRelativeRotation().Pitch + LookInput.Y) < LookPitchMax)
		{
			SpringArmComp->AddRelativeRotation(FRotator(LookInput.Y, 0.f, 0.f));
		}
	}

	if (!FMath::IsNearlyZero(LookInput.X))
	{
		SpringArmComp->AddRelativeRotation(FRotator(0.f, LookInput.X, 0.f));
	}
}

/*************** Ground Control ******************/
void AMyPawn::MoveGround(const FInputActionValue& Value)
{
	if (!Controller) return;

	FRotator CameraRotation = SpringArmComp->GetComponentRotation();
	FRotator CameraRelativeRotation = SpringArmComp->GetRelativeRotation();

	// Align pawn's yaw to the (world) direction of camera
	SetActorRotation(FRotator(0, CameraRotation.Yaw, 0));
	// maintain relative pitch of camera
	SpringArmComp->SetRelativeRotation(FRotator(CameraRelativeRotation.Pitch, 0, 0));

	FVector2D MoveInput = Value.Get<FVector2D>();
	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddActorWorldOffset(GetActorForwardVector() * MoveInput.X * GroundSpeed, true);
	}

	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddActorWorldOffset(GetActorRightVector() * MoveInput.Y * GroundSpeed, true);
	}	
}

// Take-off that allows for going into the air
void AMyPawn::MoveUp(const FInputActionValue& Value)
{
	if (!Controller) return;

	float MoveInput = Value.Get<float>();
	if (!FMath::IsNearlyZero(MoveInput))
	{
		if (MoveInput < 0)
		{
			// Cannot move downward if not in the air
			if (!bIsInTheAir) return;
		}
		AddActorWorldOffset(FVector(0, 0, MoveInput * AccelerateForce), true);
	}
}
/*************** Ground Control ******************/

/*************** Air Control *********************/
void AMyPawn::Roll(const FInputActionValue& Value)
{
	if (!Controller || !bIsInTheAir) return;
	
	bShouldResetRoll = false; // turn off roll reset in Tick

	float RollInput = Value.Get<float>();
	if (!FMath::IsNearlyZero(RollInput))
	{
		if (abs(SkeletalMeshComp->GetRelativeRotation().Roll + AirRollSpeed * RollInput) < AirRollMax)
		{
			SkeletalMeshComp->AddRelativeRotation(FRotator(0.f, 0.f, AirRollSpeed * RollInput));
		}
	}
}

// Called when the Roll trigger event is Completed
void AMyPawn::ResetRoll(const FInputActionValue& Value)
{
	if (!Controller || !bIsInTheAir) return;

	// turn on roll reset in Tick
	ResetRollAlpha = 0.f;
	ResetRollStart = SkeletalMeshComp->GetRelativeRotation().Roll;
	bShouldResetRoll = true;
}

void AMyPawn::Pitch(const FInputActionValue& Value)
{
	if (!Controller || !bIsInTheAir) return;

	bShouldResetPitch = false; // turn off pitch reset in Tick

	float PitchInput = Value.Get<float>();
	if (!FMath::IsNearlyZero(PitchInput))
	{
		if (abs(SkeletalMeshComp->GetRelativeRotation().Pitch + AirPitchSpeed * PitchInput) < AirPitchMax)
		{
			SkeletalMeshComp->AddRelativeRotation(FRotator(AirPitchSpeed * PitchInput * -1, 0.f, 0.f));
		}
	}
}

// Called when the Pitch trigger event is Completed
void AMyPawn::ResetPitch(const FInputActionValue& Value)
{
	if (!Controller || !bIsInTheAir) return;

	// turn on pitch reset in Tick
	ResetPitchAlpha = 0.f;
	ResetPitchStart = SkeletalMeshComp->GetRelativeRotation().Pitch;
	bShouldResetPitch = true;
}

void AMyPawn::Yaw(const FInputActionValue& Value)
{
	if (!Controller || !bIsInTheAir) return;

	float YawInput = Value.Get<float>();
	if (!FMath::IsNearlyZero(YawInput))
	{
		SkeletalMeshComp->AddRelativeRotation(FRotator(0.f, AirYawSpeed * YawInput, 0.f));
	}
}

// Accelerate upwards based on the current rotation of skeletal mesh
void AMyPawn::Accelerate(const FInputActionValue& Value)
{
	if (!Controller || !bIsInTheAir) return;

	float AccelerateInput = Value.Get<float>();
	if (!FMath::IsNearlyZero(AccelerateInput))
	{
		// Turn off free fall in Tick
		bShouldActivateFallTime = false;
		FallTimeElapsed = 0.f;

		// This acceleration vector is added with gravity vector in Tick to determine final orientation of the actor
		AddActorWorldOffset(AccelerateInput * SkeletalMeshComp->GetUpVector() * AccelerateForce, true);
	}
}

// Called when the Accelerate trigger event is Completed
void AMyPawn::StopAccelerate(const FInputActionValue& Value)
{
	if (!Controller || !bIsInTheAir) return;

	// Turn on free fall in Tick
	FallTimeElapsed = 0.f;
	bShouldActivateFallTime = true;
}
/*********************** Air Control ********************/

/*************** Update Conditions / Switches **************/
void AMyPawn::LineTraceFloor()
{
	// Line trace from the actor to the point past the bottom of the collision box
	FVector TraceStart = GetActorLocation();
	FVector TraceEnd = GetActorLocation() + FVector(0, 0, -1.f) * (BoxComp->GetLocalBounds().BoxExtent.Z + 15.f);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // Ignores the actor itself when detecting collisions from line tracing

	FHitResult Hit;
	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, TraceChannel);
	
	bool bHitFloor = Hit.bBlockingHit && Hit.GetActor();

	// air/ground condition is about to be flipped
	if (bIsInTheAir == bHitFloor)
	{
		// 1. turn off free fall in Tick
		FallTimeElapsed = 0.f;
		bShouldActivateFallTime = false;

		// 2. turn off reset pitch/roll in Tick
		bShouldResetPitch = false;
		bShouldResetRoll = false;
		
		// 3. reset rotation
		FRotator MeshRotation = SkeletalMeshComp->GetRelativeRotation();
		SkeletalMeshComp->SetRelativeRotation(FRotator(0.f));

		// 4. switch IMC (ground <-> flying)
		SwitchIMC(!bHitFloor);
	}

	// IsInTheAir? = the opposite of hitting the floor
	bIsInTheAir = !bHitFloor;
}

// Switch input mapping context for ground / air control
void AMyPawn::SwitchIMC(bool bShouldFly)
{
	if (AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (PlayerController->IMC_Ground && PlayerController->IMC_Flying)
				{
					// Switch IMC from ground to flying
					if (bShouldFly)
					{
						Subsystem->RemoveMappingContext(PlayerController->IMC_Ground);
						Subsystem->AddMappingContext(PlayerController->IMC_Flying, 0);
					}

					// Switch IMC from flying to ground
					else
					{
						Subsystem->RemoveMappingContext(PlayerController->IMC_Flying);
						Subsystem->AddMappingContext(PlayerController->IMC_Ground, 0);
					}
				}
			}
		}
	}
}
/*************** Update Conditions / Switches **************/


// Binding input actions to class methods
void AMyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetController()))
		{

			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&AMyPawn::Look
				);
			}

			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&AMyPawn::MoveGround
				);
			}

			if (PlayerController->MoveUpAction)
			{
				EnhancedInput->BindAction(
					PlayerController->MoveUpAction,
					ETriggerEvent::Triggered,
					this,
					&AMyPawn::MoveUp
				);
			}

			if (PlayerController->RollAction)
			{
				EnhancedInput->BindAction(
					PlayerController->RollAction,
					ETriggerEvent::Triggered,
					this,
					&AMyPawn::Roll
				);
				
				EnhancedInput->BindAction(
					PlayerController->RollAction,
					ETriggerEvent::Completed,
					this,
					&AMyPawn::ResetRoll
				);
			}

			if (PlayerController->PitchAction)
			{
				EnhancedInput->BindAction(
					PlayerController->PitchAction,
					ETriggerEvent::Triggered,
					this,
					&AMyPawn::Pitch
				);

				EnhancedInput->BindAction(
					PlayerController->PitchAction,
					ETriggerEvent::Completed,
					this,
					&AMyPawn::ResetPitch
				);
			}

			if (PlayerController->YawAction)
			{
				EnhancedInput->BindAction(
					PlayerController->YawAction,
					ETriggerEvent::Triggered,
					this,
					&AMyPawn::Yaw
				);
			}

			if (PlayerController->AccelerateAction)
			{
				EnhancedInput->BindAction(
					PlayerController->AccelerateAction,
					ETriggerEvent::Triggered,
					this,
					&AMyPawn::Accelerate
				);
				
				EnhancedInput->BindAction(
					PlayerController->AccelerateAction,
					ETriggerEvent::Completed,
					this,
					&AMyPawn::StopAccelerate
				);
			}
		}
	}

}

