// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "CharacterTypes.h"
#include "Interfaces/PickupInterface.h"
#include "SlashCharacter.generated.h"

class USlashOverlay;
class AWeapon;
class AItem;
class UCameraComponent;
class UGroomComponent;
class USpringArmComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class SLASH_API ASlashCharacter : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

public:
	ASlashCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	FORCEINLINE virtual void SetOverlappingItem(AItem* Item) override;
	FORCEINLINE virtual AItem* GetOverlappingItem() override;
	virtual void AddSouls(ASoul* Soul) override;
	virtual void AddGold(ATreasure* Treasure) override;

	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
	FORCEINLINE EDeathPose GetDeathPose() const { return DeathPose; }

protected:
	virtual void BeginPlay() override;

	/**
	 * Input callbacks
	 */
	
	void Move(const FInputActionValue& Value);
	void Turn(const FInputActionValue& Value);
	bool CanDodge();
	void Dodge();
	void EKeypressed();
	FORCEINLINE virtual bool CanAttack() override;
	virtual void Attack() override;

	/**
	 * Animation Montages
	 */
	
	void PlayEquipMontage(const FName& SectionName);
	virtual void AttackEnd() override;
	virtual void DodgeEnd() override;
	
	/// Equip weapon from back slot
	UFUNCTION(BlueprintCallable)
	void Arm();
	/// Unequip weapon to back slot
	UFUNCTION(BlueprintCallable)
	void Disarm();
	/// End equipping animation
	UFUNCTION(BlueprintCallable)
	void EndEquipping();
	/// Hit Reaction montage end
	UFUNCTION(BlueprintCallable)
	void HitReactionEnd();

	/// Handle when this character dies
	virtual void Die() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> TurnAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> EquipAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> DodgeAction;

private:

	void SetHUDHealth();
	
	/**
	 * States
	 */
	
	UPROPERTY(VisibleAnywhere)
	ECharacterState CharacterState = ECharacterState::Unequipped;
	UPROPERTY(VisibleAnywhere)
	EActionState ActionState = EActionState::Unoccupied;

	/**
	 * Components
	 */
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UGroomComponent> HairComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UGroomComponent> EyebrowsComponent;

	// Current item overlapping with character, will naturally be the closest item
	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<AItem> OverlappingItem;

	/**
	 * Animation montages
	 */
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	TObjectPtr<UAnimMontage> EquipMontage;

	/**
	 * Overlays
	 */

	UPROPERTY()
	TObjectPtr<USlashOverlay> SlashOverlay;

};
