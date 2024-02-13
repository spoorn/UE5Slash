// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SlashAnimInstance.h"

#include "Character/SlashCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void USlashAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if ((SlashCharacter = Cast<ASlashCharacter>(TryGetPawnOwner())))
	{
		SlashCharacterMovementComponent = SlashCharacter->GetCharacterMovement();
	}
}

void USlashAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (SlashCharacterMovementComponent)
	{
		GroundSpeed = UKismetMathLibrary::VSizeXY(SlashCharacterMovementComponent->Velocity);
		IsFalling = SlashCharacterMovementComponent->IsFalling();
		CharacterState = SlashCharacter->GetCharacterState();
		ActionState = SlashCharacter->GetActionState();
		DeathPose = SlashCharacter->GetDeathPose();
	}
}
