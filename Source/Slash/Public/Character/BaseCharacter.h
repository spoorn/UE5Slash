// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "BaseCharacter.generated.h"

class UAttributeComponent;
class AWeapon;

UCLASS()
class SLASH_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	// Should be created in Blueprints
	static inline FName RightHandSocketName = FName("RightHandSocket");
	static inline FName BackSocketName = FName("BackSocket");
	
	ABaseCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;

protected:
	virtual void BeginPlay() override;

	/**
	 * Weapon Hit Collision
	 */
	UFUNCTION(BlueprintCallable)
	void SetWeaponCollision(ECollisionEnabled::Type CollisionType);

	FORCEINLINE virtual bool IsAlive();
	/// True if character can attack, else false
	FORCEINLINE virtual bool CanAttack();
	/// Begin attack
	virtual void Attack();
	/// Select a random section from animation montage and play it, returning the section index
	/// returns -1 if can't play the montage
	int32 PlayRandomMontageSection(TObjectPtr<UAnimMontage> Montage);
	/// Play Attack Montage animation
	virtual void PlayAttackMontage();
	/// Stop Attack Montage animation
	virtual void StopAttackMontage();
	/// End of attack notification
	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	/// Handle taking damage
	virtual void HandleDamage(float DamageAmount);

	/// Handle when this enemy dies
	virtual void Die();

	/// Play the Hit React animation montage
	void PlayHitReactMontage(const FName& SectionName);
	/// Directional hit reaction
	void DirectionalHitReact(const FVector& ImpactPoint);
	/// Play sound for receiving a hit
	void PlayHitSound(const FVector& ImpactPoint);
	/// Spawn particles for receiving a hit
	void SpawnHitParticles(const FVector& ImpactPoint);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAttributeComponent> Attributes;

	/// Current equipped weapon
	UPROPERTY(VisibleAnywhere, Category = Weapon)
	TObjectPtr<AWeapon> EquippedWeapon;

	/// Keep track of who this enemy is in focused combat with
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat)
	TObjectPtr<AActor> CombatTarget;

	/**
	 * Animation montages
	 */
	
	/// Animation montage for attacks
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	TObjectPtr<UAnimMontage> AttackMontage;
	/// Animation montages for reacting to when getting hit
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	TObjectPtr<UAnimMontage> HitReactMontage;
	/// Animation montage on death
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	TObjectPtr<UAnimMontage> DeathMontage;

	/**
	 * For Motion Warping
	 */

	UPROPERTY(EditAnywhere, Category = Combat)
	double WarpTargetDistance = 75;

	UFUNCTION(BlueprintCallable)
	FVector GetTranslationWarpTarget();

	UFUNCTION(BlueprintCallable)
	FRotator GetRotationWarpTarget();
	
	/**
	 * Sounds
	 */
	
	/// Sound effect to play when enemy is hit by weapon
	UPROPERTY(EditAnywhere, Category = Sound)
	TObjectPtr<USoundBase> HitSound;

	/**
	 * Particles
	 */
	
	/// Hit particles, legacy Cascade system
	UPROPERTY(EditAnywhere, Category = Particles)
	TObjectPtr<UParticleSystem> HitParticles;

};
