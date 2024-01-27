// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

#include "AIController.h"
#include "Asset/AssetMacros.h"
#include "Components/AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/PawnSensingComponent.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// Don't need to attach as it has no mesh or any attachable property
	Attributes = CreateDefaultSubobject<UAttributeComponent>("Attributes");

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
}

void AEnemy::Die()
{
	// Play death montage
	if (TObjectPtr<UAnimInstance> AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
		const int32 Selection = FMath::RandRange(0, DeathMontage->GetNumSections() - 1);
		DeathPose = static_cast<EDeathPose>(Selection + 1);
		AnimInstance->Montage_JumpToSection(DeathMontage->GetSectionName(Selection), DeathMontage);
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->Deactivate();
	if (HealthBar)
	{
		HealthBar->SetVisibility(false);
	}
	SetLifeSpan(5);
}

void AEnemy::DirectionalHitReact(const FVector& ImpactPoint)
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

void AEnemy::PlayHitReactMontage(const FName& SectionName)
{
	if (TObjectPtr<UAnimInstance> AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

void AEnemy::OnPawnSeen(APawn* Pawn)
{
	if (EnemyState < EEnemyState::Chasing && Pawn->ActorHasTag(SlashCharacterTagName) && InTargetRange(Pawn, CombatRadius))
	{
		GetWorldTimerManager().ClearTimer(PatrolTimer);
		GetCharacterMovement()->MaxWalkSpeed = 300;  // Running speed
		CombatTarget = Pawn;
		EnemyState = EEnemyState::Chasing;
		MoveToTarget(CombatTarget);
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
	MoveRequest.SetAcceptanceRadius(15);
	AIController->MoveTo(MoveRequest);
}

void AEnemy::CheckCombatTarget()
{
	// Outside combat radius
	if (!InTargetRange(CombatTarget, CombatRadius))
	{
		CombatTarget = nullptr;
		if (HealthBar)
		{
			HealthBar->SetVisibility(false);
		}
		EnemyState = EEnemyState::Patrolling;
		GetCharacterMovement()->MaxWalkSpeed = 150;
		// Go back to patrolling after small delay
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, 1);
	} else if (const bool InAttackRange = InTargetRange(CombatTarget, AttackRadius); InAttackRange && EnemyState != EEnemyState::Chasing)
	{
		// Outside attack range but within combat radius, start chasing
		EnemyState = EEnemyState::Chasing;
		GetCharacterMovement()->MaxWalkSpeed = 300;
		MoveToTarget(CombatTarget);
	} else if (InAttackRange && EnemyState != EEnemyState::Attacking)
	{
		// Inside attack range, attack target
		EnemyState = EEnemyState::Attacking;
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

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget);
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (EnemyState > EEnemyState::Patrolling)
	{
		// Escalated enough to check combat target
		CheckCombatTarget();
	} else
	{
		CheckPatrolTarget();
	}
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	//DRAW_SPHERE(ImpactPoint);
	if (HealthBar)
	{
		HealthBar->SetVisibility(true);
	}
	if (Attributes && Attributes->IsAlive())
	{
		DirectionalHitReact(ImpactPoint);
	} else
	{
		Die();
	}

	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
		if (HitParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(this, HitParticles, ImpactPoint);
		}
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (Attributes)
	{
		Attributes->ReceiveDamage(DamageAmount);
		if (HealthBar)
		{
			HealthBar->SetHealthPercent(Attributes->GetHealthPercent());
		}
	}
	CombatTarget = EventInstigator->GetPawn();
	EnemyState = EEnemyState::Chasing;
	GetCharacterMovement()->MaxWalkSpeed = 300;
	MoveToTarget(CombatTarget);
	return DamageAmount;
}

