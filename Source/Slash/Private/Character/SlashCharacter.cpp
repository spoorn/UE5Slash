// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SlashCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GroomComponent.h"
#include "InputMappingContext.h"
#include "Asset/AssetMacros.h"
#include "Camera/CameraComponent.h"
#include "Components/AttributeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"
#include "Items/Soul.h"
#include "Items/Treasure/Treasure.h"
#include "Items/Weapon/Weapon.h"

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
	LOAD_ASSET_TO_VARIABLE(UInputAction, "/Game/Input/Actions/IA_Dodge", DodgeAction);

	// Animation montages
	LOAD_ASSET_TO_VARIABLE(UAnimMontage, "/Game/Blueprints/Character/Animations/AM_Attack", AttackMontage);
	LOAD_ASSET_TO_VARIABLE(UAnimMontage, "/Game/Blueprints/Character/Animations/AM_Equip", EquipMontage);
	LOAD_ASSET_TO_VARIABLE(UAnimMontage, "/Game/Blueprints/Character/Animations/AM_HitReact", HitReactMontage);
	LOAD_ASSET_TO_VARIABLE(UAnimMontage, "/Game/Blueprints/Character/Animations/AM_Death", DeathMontage);
	LOAD_ASSET_TO_VARIABLE(UAnimMontage, "/Game/Blueprints/Character/Animations/AM_Dodge", DodgeMontage);
}

void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Attributes)
	{
		Attributes -> RegenStamina(DeltaTime);
		if (SlashOverlay)
		{
			SlashOverlay->SetStaminaPercent(Attributes->GetStaminaPercent());
		}
	}
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

		if (ASlashHUD* SlashHUD = Cast<ASlashHUD>(PlayerController->GetHUD()))
		{
			if (SlashOverlay = SlashHUD->GetSlashOverlay(); SlashOverlay)
			{
				SetHUDHealth();
				SlashOverlay->SetStaminaPercent(1);
				SlashOverlay->SetGold(0);
				SlashOverlay->SetSouls(0);
			}
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

bool ASlashCharacter::CanDodge()
{
	return Attributes && Attributes->CanDodge();
}

void ASlashCharacter::Dodge()
{
	if (ActionState != EActionState::Unoccupied || !CanDodge()) return;
	PlayDodgeMontage();
	if (Attributes)
	{
		Attributes->UseStamina(Attributes->GetDodgeCost());
	}
	ActionState = EActionState::Dodge;
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

void ASlashCharacter::DodgeEnd()
{
	Super::DodgeEnd();
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

void ASlashCharacter::Die()
{
	Super::Die();
	ActionState = EActionState::Dead;
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
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Dodge);
	}
}

void ASlashCharacter::SetHUDHealth()
{
	if (SlashOverlay && Attributes)
	{
		SlashOverlay->SetHealthPercent(Attributes->GetHealthPercent());
	}
}

float ASlashCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	SetHUDHealth();
	return DamageAmount;
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	if (IsAlive())
	{
		ActionState = EActionState::HitReaction;
	}
}

void ASlashCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}

AItem* ASlashCharacter::GetOverlappingItem()
{
	return OverlappingItem;
}

void ASlashCharacter::AddSouls(ASoul* Soul)
{
	if (Attributes)
	{
		Attributes->AddSouls(Soul->GetSouls());
		if (SlashOverlay)
		{
			SlashOverlay->SetSouls(Attributes->GetSouls());
		}
	}
}

void ASlashCharacter::AddGold(ATreasure* Treasure)
{
	if (Attributes)
	{
		Attributes->AddGold(Treasure->GetGold());
		if (SlashOverlay)
		{
			SlashOverlay->SetGold(Attributes->GetGold());
		}
	}
}

