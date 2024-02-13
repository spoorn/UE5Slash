// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BaseCharacter.h"

#include "Asset/AssetMacros.h"
#include "Character/CharacterTypes.h"
#include "Components/AttributeComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Items/Weapon/Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Don't need to attach as it has no mesh or any attachable property
	Attributes = CreateDefaultSubobject<UAttributeComponent>("Attributes");

	// Always ignore camera
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// Always allow overlap events for characters
	GetMesh()->SetGenerateOverlapEvents(true);
	// Block visibility channel for hit traces
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// Set default hit sound and hit particle effect
	LOAD_ASSET_TO_VARIABLE(USoundBase, "/Game/Audio/MetaSounds/SFX_HitFlesh", HitSound);
	LOAD_ASSET_TO_VARIABLE(UParticleSystem, "/Game/VFX/Blood/Effects/ParticleSystems/Gameplay/Player/P_body_bullet_impact", HitParticles);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

bool ABaseCharacter::IsAlive()
{
	return Attributes && Attributes->IsAlive();
}

bool ABaseCharacter::CanAttack()
{
	return false;
}

void ABaseCharacter::Attack()
{
}

int32 ABaseCharacter::PlayRandomMontageSection(TObjectPtr<UAnimMontage> Montage)
{
	// TODO: Refactor this to caller to optimize
	if (TObjectPtr<UAnimInstance> AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && Montage)
	{
		// Pick animation instance at random
		const int32 Selection = FMath::RandRange(0, Montage->GetNumSections() - 1);
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(Montage->GetSectionName(Selection), Montage);
		return Selection;
	}
	return -1;
}

void ABaseCharacter::PlayAttackMontage()
{
	PlayRandomMontageSection(AttackMontage);
}

void ABaseCharacter::StopAttackMontage()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Stop(0.25, AttackMontage);
	}
}

void ABaseCharacter::PlayDeathMontage()
{
	const int32 Selection = PlayRandomMontageSection(DeathMontage);
	if (Selection < static_cast<int32>(EDeathPose::MAX))
	{
		DeathPose = static_cast<EDeathPose>(Selection);
	}
}

void ABaseCharacter::AttackEnd()
{
}

void ABaseCharacter::HandleDamage(float DamageAmount)
{
	if (Attributes)
	{
		Attributes->ReceiveDamage(DamageAmount);
	}
}

void ABaseCharacter::Die()
{
	PlayDeathMontage();
	SetWeaponCollision(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->Deactivate();
}

void ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	if (TObjectPtr<UAnimInstance> AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
{
	const FVector Forward = GetActorForwardVector();
	// Level impact with actor's Z location so debug information is visually accurate
	const FVector ImpactLeveled = FVector(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector ToImpact = (ImpactLeveled - GetActorLocation()).GetSafeNormal();
	// Get angle in degrees
	double Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Forward, ToImpact)));

	// LHR: if negative, to left, if positive, to right of actor's forward
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToImpact);

	if (CrossProduct.Z < 0)
	{
		Angle *= -1;
	}

	FName SectionName;
	if (Angle < 45 && Angle >= -45)
	{
		SectionName = FName("FromFront");
	} else if (Angle >= -135 && Angle < -45)
	{
		SectionName = FName("FromLeft");
	} else if (Angle >= 45 && Angle < 135)
	{
		SectionName = FName("FromRight");
	} else
	{
		SectionName = FName("FromBack");
	}

	PlayHitReactMontage(SectionName);
}

void ABaseCharacter::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
	}
}

void ABaseCharacter::SpawnHitParticles(const FVector& ImpactPoint)
{
	if (HitParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, HitParticles, ImpactPoint);
	}
}

FVector ABaseCharacter::GetTranslationWarpTarget()
{
	if (!CombatTarget) return FVector();

	const FVector CombatTargetLocation = CombatTarget->GetActorLocation();
	const FVector MyLocation = GetActorLocation();
	const FVector TargetToMe = MyLocation - CombatTargetLocation;
	if (TargetToMe.Length() < WarpTargetDistance)
	{
		return MyLocation;
	} else
	{
		return CombatTargetLocation + (TargetToMe.GetSafeNormal() * WarpTargetDistance);
	}
}

FRotator ABaseCharacter::GetRotationWarpTarget()
{
	if (CombatTarget)
	{
		return (CombatTarget->GetActorLocation() - GetActorLocation()).Rotation();
	}
	return FRotator();
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (IsAlive() && Hitter)
	{
		DirectionalHitReact(Hitter->GetActorLocation());
		SetWeaponCollision(ECollisionEnabled::NoCollision);
	}
	else
	{
		Die();
	}

	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);
}

void ABaseCharacter::SetWeaponCollision(ECollisionEnabled::Type CollisionType)
{
	if (EquippedWeapon && EquippedWeapon->GetCollisionBox())
	{
		EquippedWeapon->GetCollisionBox()->SetCollisionEnabled(CollisionType);
		if (CollisionType == ECollisionEnabled::NoCollision)
		{
			// If not in the middle of an attack and when collision notified to be enabled,
			// just disable overlap events to prevent double hitting enemies outside the
			// hit collision window. This is precautionary and can instead be avoided by
			// configuring the hit window and box on the weapon to be more accurate
			EquippedWeapon->GetCollisionBox()->SetGenerateOverlapEvents(false);
		} else
		{
			EquippedWeapon->GetCollisionBox()->SetGenerateOverlapEvents(true);
		}
		// Clear collision ignore actors for attack at the beginning and end of each attack
		EquippedWeapon->CollisionIgnoreActors.Empty();
	}
}

