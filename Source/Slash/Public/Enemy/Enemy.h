// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterTypes.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "Enemy.generated.h"

class AAIController;
enum class EDeathPose : uint8;
class UWidgetComponent;
class UAttributeComponent;

UCLASS()
class SLASH_API AEnemy : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	AEnemy();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;

	/// Handle when this enemy dies
	void Die();

	/// Directional hit reaction
	void DirectionalHitReact(const FVector& ImpactPoint);

	/// Play the Hit React animation montage
	void PlayHitReactMontage(const FName& SectionName);

	/// Return whether in target range of a target actor
	FORCEINLINE bool InTargetRange(TObjectPtr<AActor> Target, double Radius);

	/// Move Enemy to a target actor
	FORCEINLINE void MoveToTarget(TObjectPtr<AActor> Target);

	UPROPERTY(BlueprintReadOnly)
	EDeathPose DeathPose = EDeathPose::Alive;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAttributeComponent> Attributes;

	/// Health bar widget component
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UHealthBarComponent> HealthBar;

	/// Keep track of who this enemy is in focused combat with
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AActor> CombatTarget;

	/// Radius before losing focus on combat target
	UPROPERTY(EditAnywhere, Category = "Combat")
	double CombatRadius = 500;

	/// Radius before losing focus on patrol target
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	double PatrolRadius = 200;

	// Timer handle for wait time at patrol points
	// FTimer is a timer that holds a callback function that executes when timer is finished
	FTimerHandle PatrolTimer;
	void PatrolTimerFinished();

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
	
	/**
	 * Animation Montages
	 */
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	TObjectPtr<UAnimMontage> HitReactMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	TObjectPtr<UAnimMontage> DeathMontage;

	/// Sound effect to play when enemy is hit by weapon
	UPROPERTY(EditAnywhere, Category = Sound)
	TObjectPtr<USoundBase> HitSound;

	/// Hit particles, legacy Cascade system
	UPROPERTY(EditAnywhere, Category = Particles)
	TObjectPtr<UParticleSystem> HitParticles;
	
};
