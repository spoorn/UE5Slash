// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyTypes.h"
#include "Character/BaseCharacter.h"
#include "Character/CharacterTypes.h"
#include "Enemy.generated.h"

enum class EEnemyState : uint8;
class AAIController;
enum class EDeathPose : uint8;
class UWidgetComponent;

UCLASS()
class SLASH_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	virtual void Tick(float DeltaTime) override;
	
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;

	virtual bool CanAttack() override;
	virtual void Attack() override;
	virtual void PlayAttackMontage() override;

	virtual void HandleDamage(float DamageAmount) override;

	/// Handle when this enemy dies
	virtual void Die() override;
	/// Callback when actor is destroyed
	virtual void Destroyed() override;

	/// On pawn seen callback
	UFUNCTION()
	void OnPawnSeen(APawn* Pawn);

	/// Return whether in target range of a target actor
	FORCEINLINE bool InTargetRange(TObjectPtr<AActor> Target, double Radius);

	/// Move Enemy to a target actor
	FORCEINLINE void MoveToTarget(TObjectPtr<AActor> Target);

	/// Check if combat target should change
	FORCEINLINE void CheckCombatTarget();
	/// Check if patrol target should change
	FORCEINLINE void CheckPatrolTarget();

	UPROPERTY(BlueprintReadOnly)
	EDeathPose DeathPose;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::Patrolling;

	UPROPERTY(EditAnywhere, Category = Combat)
	float DeathLifeSpan = 8;

private:
	/**
	 * AI Behavior
	 */

	/// Hide health bar widget
	void HideHealthBar();
	/// Show health bar widget
	void ShowHealthBar();
	/// Lose interest in the combat target
	void LoseInterest();
	/// Start patrolling action
	void StartPatrolling();
	/// Chase combat target
	void ChaseTarget();
	/// Check if enemy is outside of its combat radius with CombatTarget
	bool IsOutsideCombatRadius();
	/// Check if enemy is within its attack radius of Combat Target
	bool IsInAttackRadius();
	/// Check if in chasing state
	bool IsChasing();
	/// Check if enemy is attacking
	bool IsAttacking();
	/// Check if enemy is dead
	bool IsDead();
	/// Check if enemy is engaged in combat
	bool IsEngaged();


	/**
	 * Combat
	 */

	/// Timer handle for how long enemy stays in an Attacking state
	FTimerHandle AttackTimer;
	UFUNCTION()
	void StartAttackTimer();
	/// Cancel the patrol timer
	void ClearAttackTimer();

	/// Minimum time in seconds of attack timer
	UPROPERTY(EditAnywhere, Category = Combat)
	float AttackMin = 0.5;
	/// Max time in seconds of attack timer
	UPROPERTY(EditAnywhere, Category = Combat)
	float AttackMax = 1;
	
	
	UPROPERTY(EditAnywhere, Category = Combat)
	float PatrollingSpeed = 125;
	UPROPERTY(EditAnywhere, Category = Combat)
	float ChasingSpeed = 300;

	
	/**
	 * Components
	 */

	/// Health bar widget component
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UHealthBarComponent> HealthBar;

	/// Pawn sensor
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UPawnSensingComponent> PawnSensingComponent;

	/// Weapon the Enemy can equip
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<AWeapon> WeaponClass;

	/// Keep track of who this enemy is in focused combat with
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AActor> CombatTarget;

	/// Radius before losing focus on combat target
	UPROPERTY(EditAnywhere, Category = "Combat")
	double CombatRadius = 500;

	/// Radius for attack radius, within combat radius
	UPROPERTY(EditAnywhere, Category = "Combat")
	double AttackRadius = 140;

	/// Radius before losing focus on patrol target
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	double PatrolRadius = 200;

	// Timer handle for wait time at patrol points
	// FTimer is a timer that holds a callback function that executes when timer is finished
	FTimerHandle PatrolTimer;
	UFUNCTION()
	void PatrolTimerFinished();
	/// Cancel the patrol timer
	void ClearPatrolTimer();

	/// Min wait time in seconds for switching patrol targets
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float PatrolWaitMin = 5;
	/// Max wait time in seconds for switching patrol targets
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float PatrolWaitMax = 10;

	/**
	 * Navigation
	 */

	UPROPERTY()
	TObjectPtr<AAIController> AIController;

	// Current patrol target
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TObjectPtr<AActor> PatrolTarget;

	// Possible patrol targets
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<TObjectPtr<AActor>> PatrolTargets;
	
};
