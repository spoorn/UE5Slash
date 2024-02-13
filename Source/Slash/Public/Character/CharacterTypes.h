/**
 * Common Character enum types
 */

#pragma once

inline static FName SlashCharacterTag = FName("SlashCharacter");
inline static FName EngageableActorTagName = FName("EngageableActor");
inline static FName DeadTag = FName("Dead");

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
	HitReaction,
	Attacking,
	EquippingWeapon,
	Dead
};

/// Death pose state
UENUM(BlueprintType)
enum class EDeathPose : uint8
{
	Death1,
	Death2,
	Death3,
	Death4,
	Death5,
	Death6,
	MAX UMETA(Hidden)
};