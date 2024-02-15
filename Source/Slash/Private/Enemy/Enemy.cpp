// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

#include "AIController.h"
#include "Character/CharacterTypes.h"
#include "Components/AttributeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Items/Soul.h"
#include "Items/Weapon/Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/PawnSensingComponent.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// Enemy mesh should be WorldDynamic to have collision with player weapons
	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);

	// Set default mesh
	if (const ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMesh(TEXT("/Game/Mixamo/Paladin/SKM_Paladin"))
		; SkeletalMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMeshAsset(SkeletalMesh.Object);
		// Match default mesh with capsule
		GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));
		GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	}

	HealthBar = CreateDefaultSubobject<UHealthBarComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());
	HealthBar->SetWidgetSpace(EWidgetSpace::Screen);

	// Limit walk speed to be slower than character
	GetCharacterMovement()->MaxWalkSpeed = 150; // Default to walking speed only
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Pawn sensing
	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>("PawnSensingComponent");
	PawnSensingComponent->SightRadius = 4000;
	PawnSensingComponent->SetPeripheralVisionAngle(45);
	PawnSensingComponent->bOnlySensePlayers = false;

	// Default soul
	SoulClass = ASoul::StaticClass();
}


void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsDead()) return;
	if (EnemyState > EEnemyState::Patrolling)
	{
		// Escalated enough to check combat target
		CheckCombatTarget();
	}
	else
	{
		CheckPatrolTarget();
	}
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	ShowHealthBar();
	ClearPatrolTimer();
	ClearAttackTimer();
	if (IsEngaged())
	{
		StopAttackMontage();
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	CombatTarget = EventInstigator->GetPawn();
	if (IsInAttackRadius())
	{
		EnemyState = EEnemyState::Attacking;
	} else
	{
		ChaseTarget();
	}
	return DamageAmount;
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	Tags.Add(EnemyTag);

	HideHealthBar();
	AIController = Cast<AAIController>(GetController());
	MoveToTarget(PatrolTarget);

	if (PawnSensingComponent)
	{
		PawnSensingComponent->OnSeePawn.AddDynamic(this, &AEnemy::OnPawnSeen);
	}

	if (Attributes)
	{
		// Random soul amount
		Attributes->AddSouls(FMath::RandRange(1, 10));
	}

	SpawnDefaultWeapon();
}

void AEnemy::SpawnDefaultWeapon()
{
	if (UWorld* World = GetWorld(); World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), PrimaryWeaponSocketName, this, this);
		EquippedWeapon = DefaultWeapon;
	}
}

bool AEnemy::CanAttack()
{
	return IsInAttackRadius() && !IsAttacking() && !IsEngaged() && !IsDead();
}

void AEnemy::Attack()
{
	Super::Attack();
	if (!CombatTarget) return;
	EnemyState = EEnemyState::Engaged;
	PlayAttackMontage();
}

void AEnemy::AttackEnd()
{
	Super::AttackEnd();
	EnemyState = EEnemyState::NoState;
	CheckCombatTarget();
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);

	if (Attributes && HealthBar)
	{
		HealthBar->SetHealthPercent(Attributes->GetHealthPercent());
	}
}

void AEnemy::SpawnSoul()
{
	if (UWorld* World = GetWorld(); World && SoulClass)
	{
		if (ASoul* Soul = Cast<ASoul>(UGameplayStatics::BeginDeferredActorSpawnFromClass(World, SoulClass, GetActorTransform())))
		{
			// Drop all souls
			Soul->SetSouls(Attributes->GetSouls());
			UGameplayStatics::FinishSpawningActor(Soul, GetActorTransform());
		}
	}
}

void AEnemy::Die()
{
	Super::Die();
	EnemyState = EEnemyState::Dead;
	ClearAttackTimer();
	HideHealthBar();
	SetLifeSpan(DeathLifeSpan);
	SpawnSoul();
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
		&& Pawn->ActorHasTag(EngageableActorTagName);

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
	}
	else if (!IsInAttackRadius() && !IsChasing())
	{
		ClearAttackTimer();
		if (!IsEngaged())
		{
			ChaseTarget();
		}
	}
	else if (CanAttack())
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
	if (!IsDead() && HealthBar)
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
