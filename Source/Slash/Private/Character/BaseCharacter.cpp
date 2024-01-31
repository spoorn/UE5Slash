// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BaseCharacter.h"

#include "Components/AttributeComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Items/Weapon/Weapon.h"
#include "Kismet/GameplayStatics.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Don't need to attach as it has no mesh or any attachable property
	Attributes = CreateDefaultSubobject<UAttributeComponent>("Attributes");
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
		SectionName = FName("FromBack");
	} else if (Angle >= 45 && Angle < 135)
	{
		SectionName = FName("FromRight");
	} else
	{
		SectionName = FName("FromLeft");
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

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseCharacter::SetWeaponCollision(ECollisionEnabled::Type CollisionType)
{
	if (EquippedWeapon && EquippedWeapon->GetCollisionBox())
	{
		EquippedWeapon->GetCollisionBox()->SetCollisionEnabled(CollisionType);
		// Clear collision ignore actors for attack at the beginning and end of each attack
		EquippedWeapon->CollisionIgnoreActors.Empty();
	}
}
