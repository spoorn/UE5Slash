/**
 * Common Character enum types
 */

#pragma once

/// Character types driving animations
UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	Unequipped UMETA(DisplayName = "Unequipped"),
	EquippedOneHandedWeapon UMETA(DisplayName = "Equipped One-Handed Weapon")
};

/// Action states
UENUM(BlueprintType)
enum class EActionState : uint8
{
	Unoccupied,
	Attacking,
	EquippingWeapon
};

/// Death pose state
UENUM(BlueprintType)
enum class EDeathPose : uint8
{
	Alive,
	Death1,
	Death2,
	Death3,
	Death4,
	Death5,
	Death6
};