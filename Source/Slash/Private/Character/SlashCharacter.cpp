// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SlashCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GroomComponent.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Disable controller rotation on character to prevent sliding behavior, only used for camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Groom components
	HairComponent = CreateDefaultSubobject<UGroomComponent>("SlashHairComponent");
	HairComponent->SetupAttachment(GetMesh());
	// Attach to head socket of mesh
	HairComponent->AttachmentName = FString("head");
	EyebrowsComponent = CreateDefaultSubobject<UGroomComponent>("SlashEyebrowsComponent");
	EyebrowsComponent->SetupAttachment(GetMesh());
	EyebrowsComponent->AttachmentName = FString("head");

	// Set default mesh
	if (const ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMesh(TEXT("/Game/AncientContent/Characters/Echo/Meshes/Echo"))
		; SkeletalMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMeshAsset(SkeletalMesh.Object);
		// Match default mesh with capsule
		GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));
		GetMesh()->SetRelativeLocation(FVector(0, 0, -90));

		// Set default hair and eyebrow assets
		if (const ConstructorHelpers::FObjectFinder<UGroomAsset> GroomAsset(TEXT("/Game/AncientContent/Characters/Echo/Hair/Hair_S_UpdoBuns"))
		; GroomAsset.Succeeded())
		{
			HairComponent->SetGroomAsset(GroomAsset.Object);
		}

		if (const ConstructorHelpers::FObjectFinder<UGroomAsset> GroomAsset(TEXT("/Game/AncientContent/Characters/Echo/Hair/Eyebrows_L_Echo"))
		; GroomAsset.Succeeded())
		{
			EyebrowsComponent->SetGroomAsset(GroomAsset.Object);
		}
	}

	// Spring Arm and Camera
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SlashSpringArmComponent");
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->TargetArmLength = 300;
	SpringArmComponent->SetRelativeRotation(FRotator(-40, 0, 0));
	// Camera follows mouse
	SpringArmComponent->bUsePawnControlRotation = true;
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>("SlashCameraComponent");
	CameraComponent->SetupAttachment(SpringArmComponent);

	// Orient character to movement rotation
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0, 400, 0);
	

	// Fetch data assets from default locations if available
	if (const ConstructorHelpers::FObjectFinder<UInputMappingContext> InputMappingAsset(TEXT("/Game/Input/IMC_SlashContext"))
		; InputMappingAsset.Succeeded())
	{
		MappingContext = InputMappingAsset.Object;
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

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}
}

void ASlashCharacter::Move(const FInputActionValue& Value)
{
	if (GetController())
	{
		// Rotation of controller
		const FRotator ControlRotation = GetControlRotation();
		// Only care about yaw rotation as we only go parallel to bottom plane]
		const FRotator YawRotation{0, ControlRotation.Yaw, 0};

		// Get forward direction of controller
		// take unit vector, rotate it according to YawRotation, and extract the forward X direction
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector2D MoveValue = Value.Get<FVector2D>();
		AddMovementInput(ForwardDirection, MoveValue.Y);

		// In UE5, Y axis is right direction
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(RightDirection, MoveValue.X);
	}
}

void ASlashCharacter::Turn(const FInputActionValue& Value)
{
	if (GetController())
	{
		const FVector2D DirectionValue = Value.Get<FVector2D>();
		AddControllerYawInput(DirectionValue.X);
		AddControllerPitchInput(DirectionValue.Y);
	}
}

void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Turn);
	}
}

