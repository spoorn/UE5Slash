// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/Bird.h"

#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"

ABird::ABird()
{
	PrimaryActorTick.bCanEverTick = true;

	// Use controller pitch and yaw as we have mouse controls
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("BirdCapsuleComponent");\
	CapsuleComponent->SetCapsuleHalfHeight(20);
	CapsuleComponent->SetCapsuleRadius(15);
	SetRootComponent(CapsuleComponent);
	BirdMesh = CreateDefaultSubobject<USkeletalMeshComponent>("BirdMeshComponent");
	BirdMesh->SetupAttachment(RootComponent);
	// Set default mesh
	if (const ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMesh(TEXT("/Game/AnimalVarietyPack/Crow/Meshes/SK_Crow"))
		; SkeletalMesh.Succeeded())
	{
		BirdMesh->SetSkeletalMeshAsset(SkeletalMesh.Object);
		// Match default mesh with capsule
		BirdMesh->SetRelativeRotation(FRotator(0, -90, 0));
		BirdMesh->SetRelativeLocation(FVector(0, 0, -15));
	}

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>("BirdMovementComponent");
	
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("BirdSpringArmComponent");
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->TargetArmLength = 300;
	SpringArmComponent->SetRelativeRotation(FRotator(-15, 0, 0));
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>("BirdCameraComponent");
	CameraComponent->SetupAttachment(SpringArmComponent);

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Fetch data assets from default locations if available
	// https://docs.unrealengine.com/5.3/en-US/referencing-assets-in-unreal-engine/
	// Make sure to import InputMappingContext.h
	if (const ConstructorHelpers::FObjectFinder<UInputMappingContext> InputMappingAsset(TEXT("/Game/Input/IMC_BirdContext"))
		; InputMappingAsset.Succeeded())
	{
		BirdMappingContext = InputMappingAsset.Object;
	}
	if (const ConstructorHelpers::FObjectFinder<UInputAction> SetDestinationActionAsset(
		TEXT("/Game/Input/Actions/IA_Move")); SetDestinationActionAsset.Succeeded())
	{
		MoveAction = SetDestinationActionAsset.Object;
	}
	if (const ConstructorHelpers::FObjectFinder<UInputAction> SetDestinationActionAsset(
		TEXT("/Game/Input/Actions/IA_Turn")); SetDestinationActionAsset.Succeeded())
	{
		TurnAction = SetDestinationActionAsset.Object;
	}
}

void ABird::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(BirdMappingContext, 0);
		}
	}
}

void ABird::Move(const FInputActionValue& Value)
{
	if (const float DirectionValue = Value.Get<float>(); Controller && DirectionValue != 0)
	{
		AddMovementInput(GetActorForwardVector(), DirectionValue);
	}
}

void ABird::Turn(const FInputActionValue& Value)
{
	if (GetController())
	{
		const FVector2D DirectionValue = Value.Get<FVector2D>();
		AddControllerYawInput(DirectionValue.X);
		AddControllerPitchInput(DirectionValue.Y);
	}
}

void ABird::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABird::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABird::Move);
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ABird::Turn);
	}
}

