// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SlashCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GroomComponent.h"
#include "InputMappingContext.h"
#include "Asset/AssetMacros.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Items/Weapon/Weapon.h"

ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

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
		LOAD_ASSET_TO_CALLBACK(UGroomAsset, "/Game/AncientContent/Characters/Echo/Hair/Hair_S_UpdoBuns", HairComponent->SetGroomAsset);
		LOAD_ASSET_TO_CALLBACK(UGroomAsset, "/Game/AncientContent/Characters/Echo/Hair/Eyebrows_L_Echo", EyebrowsComponent->SetGroomAsset);
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
	LOAD_ASSET_TO_VARIABLE(UInputMappingContext, "/Game/Input/IMC_SlashContext", MappingContext);
	LOAD_ASSET_TO_VARIABLE(UInputAction, "/Game/Input/Actions/IA_Move", MoveAction);
	LOAD_ASSET_TO_VARIABLE(UInputAction, "/Game/Input/Actions/IA_Turn", TurnAction);
	LOAD_ASSET_TO_VARIABLE(UInputAction, "/Game/Input/Actions/IA_Jump", JumpAction);
	LOAD_ASSET_TO_VARIABLE(UInputAction, "/Game/Input/Actions/IA_Equip", EquipAction);
	LOAD_ASSET_TO_VARIABLE(UInputAction, "/Game/Input/Actions/IA_Attack", AttackAction);

	// Animation montages
	LOAD_ASSET_TO_VARIABLE(UAnimMontage, "/Game/Blueprints/Character/Animations/AM_Attack", AttackMontage);
	LOAD_ASSET_TO_VARIABLE(UAnimMontage, "/Game/Blueprints/Character/Animations/AM_Equip", EquipMontage);
}

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();

	Tags.Add(SlashCharacterTag);
	Tags.Add(EngageableActorTagName);

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
	if (ActionState != EActionState::Unoccupied) return;
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

void ASlashCharacter::EKeypressed()
{
	// Attach weapon to SlashCharacter's right hand socket
	if (TObjectPtr<AWeapon> Weapon = Cast<AWeapon>(OverlappingItem))
	{
		Weapon->Equip(GetMesh(), RightHandSocketName, this, this);
		CharacterState = ECharacterState::EquippedOneHandedWeapon;
		OverlappingItem = nullptr;
		EquippedWeapon = Weapon;
	} else
	{
		// Play animation montage and change state for un/equipping weapons
		if (ActionState == EActionState::Unoccupied && CharacterState != ECharacterState::Unequipped && EquippedWeapon)
		{
			PlayEquipMontage(FName("Unequip"));
			CharacterState = ECharacterState::Unequipped;
			ActionState = EActionState::EquippingWeapon;
		} else if (ActionState == EActionState::Unoccupied && CharacterState == ECharacterState::Unequipped && EquippedWeapon)
		{
			PlayEquipMontage(FName("Equip"));
			CharacterState = ECharacterState::EquippedOneHandedWeapon;
			ActionState = EActionState::EquippingWeapon;
		}
	}
}

bool ASlashCharacter::CanAttack()
{
	return ActionState == EActionState::Unoccupied && CharacterState != ECharacterState::Unequipped;
}

void ASlashCharacter::Attack()
{
	Super::Attack();
	if (CanAttack())
	{
		PlayAttackMontage();
		ActionState = EActionState::Attacking;
	}
}

void ASlashCharacter::PlayEquipMontage(const FName& SectionName)
{
	if (TObjectPtr<UAnimInstance> AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::Unoccupied;
}

void ASlashCharacter::Arm()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToComponent(GetMesh(), RightHandSocketName);
		EquippedWeapon->PlayEquipSound();
	}
}

void ASlashCharacter::Disarm()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToComponent(GetMesh(), BackSocketName);
	}
}

void ASlashCharacter::EndEquipping()
{
	ActionState = EActionState::Unoccupied;
}

void ASlashCharacter::HitReactionEnd()
{
	ActionState = EActionState::Unoccupied;
}

void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Turn);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ASlashCharacter::EKeypressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Attack);
	}
}

float ASlashCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	return DamageAmount;
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	ActionState = EActionState::HitReaction;
}

