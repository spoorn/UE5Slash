// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterTypes.h"
#include "Animation/AnimInstance.h"
#include "SlashAnimInstance.generated.h"

enum class ECharacterState : uint8;
/**
 * 
 */
UCLASS()
class SLASH_API USlashAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// Pointer to attached Character
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class ASlashCharacter> SlashCharacter;

	// Character movement component which should be on our slash character
	UPROPERTY(BlueprintReadOnly, Category = Movement)
	TObjectPtr<class UCharacterMovementComponent> SlashCharacterMovementComponent;

	// Ground speed to drive animation state
	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float GroundSpeed;

	// Is in the air
	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool IsFalling;

	// Character state
	UPROPERTY(BlueprintReadOnly, Category = "Character State")
	ECharacterState CharacterState;
};
