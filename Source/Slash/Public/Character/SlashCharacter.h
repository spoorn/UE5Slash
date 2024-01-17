// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterTypes.h"
#include "GameFramework/Character.h"
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
class SLASH_API ASlashCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASlashCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE TObjectPtr<AItem> GetOverlappingItem() { return OverlappingItem; }
	FORCEINLINE void SetOverlappingItem(TObjectPtr<AItem> Item) { OverlappingItem = Item; }
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }

	/**
	 * Weapon Hit Collision
	 */
	UFUNCTION(BlueprintCallable)
	void SetWeaponCollision(ECollisionEnabled::Type CollisionType);

protected:
	virtual void BeginPlay() override;

	/**
	 * Input callbacks
	 */
	
	void Move(const FInputActionValue& Value);
	void Turn(const FInputActionValue& Value);
	void EKeypressed();
	void Attack();

	/**
	 * Animation Montages
	 */
	
	void PlayEquipMontage(const FName& SectionName);
	void PlayAttackMontage();
	
	/// End of attack notification
	UFUNCTION(BlueprintCallable)
	void AttackEnd();
	/// Equip weapon from back slot
	UFUNCTION(BlueprintCallable)
	void Arm();
	/// Unequip weapon to back slot
	UFUNCTION(BlueprintCallable)
	void Disarm();
	/// End equipping animation
	UFUNCTION(BlueprintCallable)
	void EndEquipping();

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
	// Should be created in Blueprints
	static inline FName RightHandSocketName = FName("RightHandSocket");
	static inline FName BackSocketName = FName("BackSocket");

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

	UPROPERTY(VisibleAnywhere, Category = Weapon)
	TObjectPtr<AWeapon> EquippedWeapon;

	/**
	 * Animation montages
	 */
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	TObjectPtr<UAnimMontage> AttackMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	TObjectPtr<UAnimMontage> EquipMontage;

};
