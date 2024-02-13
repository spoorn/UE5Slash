#pragma once

#include "UObject/Class.h"

/// Tag indicating enemy actors
inline static FName EnemyTag = FName("Enemy");

/// Action states
UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	/// Not in any "active" state, for resetting
	/// Have this be the first so ABP animation defaults to this instead of Dead
	NoState,
	Dead,
	Patrolling,
	Chasing,
	/// Start phase of attacking
	Attacking,
	/// Actually engaged in combat i.e. swinging weapon
	Engaged,
};