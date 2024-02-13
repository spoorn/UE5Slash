// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "CharacterTypes.h"
#include "SlashCharacter.generated.h"

class AWeapon;
class AItem;
class UCameraComponent;
class UGroomComponent;
class USpringArmComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class SLASH_API ASlashCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	ASlashCharacter();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;

	FORCEINLINE TObjectPtr<AItem> GetOverlappingItem() { return OverlappingItem; }
	FORCEINLINE void SetOverlappingItem(TObjectPtr<AItem> Item) { OverlappingItem = Item; }
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }

protected:
	virtual void BeginPlay() override;

	/**
	 * Input callbacks
	 */
	
	void Move(const FInputActionValue& Value);
	void Turn(const FInputActionValue& Value);
	void EKeypressed();
	FORCEINLINE virtual bool CanAttack() override;
	virtual void Attack() override;

	/**
	 * Animation Montages
	 */
	
	void PlayEquipMontage(const FName& SectionName);
	virtual void AttackEnd() override;
	
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

private:

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

};
