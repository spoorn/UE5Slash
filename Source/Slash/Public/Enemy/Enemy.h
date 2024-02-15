// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyTypes.h"
#include "Character/BaseCharacter.h"
#include "Enemy.generated.h"

class ASoul;
enum class EEnemyState : uint8;
class AAIController;
class UWidgetComponent;

UCLASS()
class SLASH_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	
protected:
	virtual void BeginPlay() override;

	virtual bool CanAttack() override;
	virtual void Attack() override;
	virtual void AttackEnd() override;

	virtual void HandleDamage(float DamageAmount) override;
	void SpawnSoul();

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::Patrolling;

private:
	void SpawnDefaultWeapon();
	
	/**
	 * AI Behavior
	 */

	/// Hide health bar widget
	FORCEINLINE void HideHealthBar();
	/// Show health bar widget
	FORCEINLINE void ShowHealthBar();
	/// Lose interest in the combat target
	void LoseInterest();
	/// Start patrolling action
	void StartPatrolling();
	/// Chase combat target
	void ChaseTarget();
	/// Check if enemy is outside of its combat radius with CombatTarget
	FORCEINLINE bool IsOutsideCombatRadius();
	/// Check if enemy is within its attack radius of Combat Target
	FORCEINLINE bool IsInAttackRadius();
	/// Check if in chasing state
	FORCEINLINE bool IsChasing();
	/// Check if enemy is attacking
	FORCEINLINE bool IsAttacking();
	/// Check if enemy is dead
	FORCEINLINE bool IsDead();
	/// Check if enemy is engaged in combat
	FORCEINLINE bool IsEngaged();
	/// Check if combat target should change
	FORCEINLINE void CheckCombatTarget();
	/// Check if patrol target should change
	FORCEINLINE void CheckPatrolTarget();


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
	float AttackMin = 0.4;
	/// Max time in seconds of attack timer
	UPROPERTY(EditAnywhere, Category = Combat)
	float AttackMax = 0.7;
	
	
	UPROPERTY(EditAnywhere, Category = Combat)
	float PatrollingSpeed = 125;
	UPROPERTY(EditAnywhere, Category = Combat)
	float ChasingSpeed = 300;

	UPROPERTY(EditAnywhere, Category = Combat)
	float DeathLifeSpan = 8;

	
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
	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<AWeapon> WeaponClass;

	/// Radius before losing focus on combat target
	UPROPERTY(EditAnywhere, Category = "Combat")
	double CombatRadius = 1000;

	/// Radius for attack radius, within combat radius
	UPROPERTY(EditAnywhere, Category = "Combat")
	double AttackRadius = 200;

	/// Radius before losing focus on patrol target
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	double PatrolRadius = 200;

	/// Radius before losing focus on patrol target
	UPROPERTY(EditAnywhere, Category = "Combat")
	double AcceptanceRadius = 50;

	/// Soul class to spawn on death
	UPROPERTY(EditAnywhere, Category = "Loot")
	TSubclassOf<ASoul> SoulClass;

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
