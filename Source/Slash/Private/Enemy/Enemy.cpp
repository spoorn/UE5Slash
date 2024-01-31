// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

#include "AIController.h"
#include "Asset/AssetMacros.h"
#include "Components/AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Items/Weapon/Weapon.h"
#include "Particles/ParticleSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/PawnSensingComponent.h"

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

	HealthBar = CreateDefaultSubobject<UHealthBarComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());
	HealthBar->SetWidgetSpace(EWidgetSpace::Screen);

	// Limit walk speed to be slower than character
	GetCharacterMovement()->MaxWalkSpeed = 150;  // Default to walking speed only
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Pawn sensing
	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>("PawnSensingComponent");
	PawnSensingComponent->SightRadius = 4000;
	PawnSensingComponent->SetPeripheralVisionAngle(45);
	PawnSensingComponent->bOnlySensePlayers = false;
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (HealthBar)
	{
		HealthBar->SetHealthPercent(1);
		HealthBar->SetVisibility(false);
	}

	AIController = Cast<AAIController>(GetController());
	MoveToTarget(PatrolTarget);

	if (PawnSensingComponent)
	{
		PawnSensingComponent->OnSeePawn.AddDynamic(this, &AEnemy::OnPawnSeen);
	}

	if (UWorld* World = GetWorld(); World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), RightHandSocketName, this, this);
		EquippedWeapon = DefaultWeapon;
	}
}

bool AEnemy::CanAttack()
{
	return IsInAttackRadius() && !IsAttacking() && !IsDead();
}

void AEnemy::Attack()
{
	Super::Attack();
	PlayAttackMontage();
}

void AEnemy::PlayAttackMontage()
{
	Super::PlayAttackMontage();
	PlayRandomMontageSection(AttackMontage);
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);

	if (Attributes && HealthBar)
	{
		HealthBar->SetHealthPercent(Attributes->GetHealthPercent());
	}
}

void AEnemy::Die()
{
	Super::Die();
	EnemyState = EEnemyState::Dead;
	// Play death montage
	const int32 Selection = PlayRandomMontageSection(DeathMontage);
	if (Selection < static_cast<int32>(EDeathPose::MAX))
	{
		DeathPose = static_cast<EDeathPose>(Selection);
	}
	ClearAttackTimer();
	HideHealthBar();
	SetLifeSpan(DeathLifeSpan);
}

void AEnemy::Destroyed()
{
	Super::Destroyed();
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}

void AEnemy::OnPawnSeen(APawn* Pawn)
{
	const bool bShouldChaseTarget =
		EnemyState != EEnemyState::Dead
		&& EnemyState != EEnemyState::Chasing
		&& EnemyState < EEnemyState::Chasing
		&& Pawn->ActorHasTag(SlashCharacterTagName);

	if (bShouldChaseTarget)
	{
		CombatTarget = Pawn;
		ClearPatrolTimer();
		ChaseTarget();
	}
}

bool AEnemy::InTargetRange(TObjectPtr<AActor> Target, double Radius)
{
	return Target && (Target->GetActorLocation() - GetActorLocation()).Size() <= Radius;
}

void AEnemy::MoveToTarget(TObjectPtr<AActor> Target)
{
	if (!AIController || !Target) return;
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(50);
	AIController->MoveTo(MoveRequest);
}

void AEnemy::CheckCombatTarget()
{
	// Outside combat radius
	if (IsOutsideCombatRadius())
	{
		ClearAttackTimer();
		LoseInterest();
		if (!IsEngaged())
		{
			StartPatrolling();
		}
	} else if (!IsInAttackRadius() && !IsChasing())
	{
		ClearAttackTimer();
		if (!IsEngaged())
		{
			ChaseTarget();
		}
	} else if (CanAttack())
	{
		// Inside attack range, attack target
		StartAttackTimer();
	}
}

void AEnemy::CheckPatrolTarget()
{
	if (EnemyState != EEnemyState::Patrolling) return;
	// Patrol between targets
	// When within range of patrol target, switch targets
	if (AIController && (InTargetRange(PatrolTarget, PatrolRadius)))
	{
		TArray<TObjectPtr<AActor>> ValidTargets;
		for (auto Target : PatrolTargets)
		{
			if (Target != PatrolTarget)
			{
				ValidTargets.Add(Target);
			}
		}
		
		const int32 NumTargets = ValidTargets.Num();
		if (NumTargets > 0)
		{
			const int32 Selection = FMath::RandRange(0, NumTargets - 1);
			PatrolTarget = ValidTargets[Selection];
			// Add delay for patrolling
			const int32 RandomDelay = FMath::RandRange(PatrolWaitMin, PatrolWaitMax);
			GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, RandomDelay);
		}
	}
}

void AEnemy::HideHealthBar()
{
	if (HealthBar)
	{
		HealthBar->SetVisibility(false);
	}
}

void AEnemy::ShowHealthBar()
{
	if (HealthBar)
	{
		HealthBar->SetVisibility(true);
	}
}

void AEnemy::LoseInterest()
{
	CombatTarget = nullptr;
	HideHealthBar();
}

void AEnemy::StartPatrolling()
{
	EnemyState = EEnemyState::Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	// Go back to patrolling after small delay
	GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, 1);
}

void AEnemy::ChaseTarget()
{
	// Outside attack range but within combat radius, start chasing
	EnemyState = EEnemyState::Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(CombatTarget);
}

bool AEnemy::IsOutsideCombatRadius()
{
	return !InTargetRange(CombatTarget, CombatRadius);
}

bool AEnemy::IsInAttackRadius()
{
	return InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsChasing()
{
	return EnemyState == EEnemyState::Chasing;
}

bool AEnemy::IsAttacking()
{
	return EnemyState == EEnemyState::Attacking;
}

bool AEnemy::IsDead()
{
	return EnemyState == EEnemyState::Dead;
}

bool AEnemy::IsEngaged()
{
	return EnemyState == EEnemyState::Engaged;
}

void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemy::StartAttackTimer()
{
	EnemyState = EEnemyState::Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}

void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget);
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead()) return;
	if (EnemyState > EEnemyState::Patrolling)
	{
		// Escalated enough to check combat target
		CheckCombatTarget();
	} else
	{
		CheckPatrolTarget();
	}
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	ShowHealthBar();
	if (IsAlive())
	{
		DirectionalHitReact(ImpactPoint);
	} else
	{
		Die();
	}

	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	CombatTarget = EventInstigator->GetPawn();
	ChaseTarget();
	return DamageAmount;
}

