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
	ABaseCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/**
	 * Weapon Hit Collision
	 */
	UFUNCTION(BlueprintCallable)
	void SetWeaponCollision(ECollisionEnabled::Type CollisionType);

protected:
	virtual void BeginPlay() override;
	/// True if character can attack, else false
	virtual bool CanAttack();
	/// Begin attack
	virtual void Attack();
	/// Play Attack Montage animation
	virtual void PlayAttackMontage();
	/// End of attack notification
	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	/// Handle when this enemy dies
	virtual void Die();

	/// Play the Hit React animation montage
	void PlayHitReactMontage(const FName& SectionName);
	/// Directional hit reaction
	void DirectionalHitReact(const FVector& ImpactPoint);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAttributeComponent> Attributes;

	/// Current equipped weapon
	UPROPERTY(VisibleAnywhere, Category = Weapon)
	TObjectPtr<AWeapon> EquippedWeapon;

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
