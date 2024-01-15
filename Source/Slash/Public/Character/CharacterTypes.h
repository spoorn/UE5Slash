/**
 * Common Character enum types
 */

#pragma once

/// Character types
UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	Unequipped UMETA(DisplayName = "Unequipped"),
	EquippedOneHandedWeapon UMETA(DisplayName = "Equipped One-Handed Weapon")
};