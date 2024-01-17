// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

#include "Asset/AssetMacros.h"
#include "Components/CapsuleComponent.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// Enemy mesh should be WorldDynamic to have collision with player weapons
	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	// Block visibility channel for hit traces
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// Set default mesh
	if (const ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMesh(TEXT("/Game/Mixamo/Paladin/SKM_Paladin"))
		; SkeletalMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMeshAsset(SkeletalMesh.Object);
		// Match default mesh with capsule
		GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));
		GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	}

	// Set default hit sound and hit particle effect
	LOAD_ASSET_TO_VARIABLE(USoundBase, "/Game/Audio/MetaSounds/SFX_HitFlesh", HitSound);
	LOAD_ASSET_TO_VARIABLE(UParticleSystem, "/Game/VFX/Blood/Effects/ParticleSystems/Gameplay/Player/P_body_bullet_impact", HitParticles);
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}

void AEnemy::PlayHitReactMontage(const FName& SectionName)
{
	if (TObjectPtr<UAnimInstance> AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::GetHit(const FVector& ImpactPoint)
{
	//DRAW_SPHERE(ImpactPoint);
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

	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
		if (HitParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(this, HitParticles, ImpactPoint);
		}
	}
}

