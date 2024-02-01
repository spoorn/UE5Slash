#pragma once

#include "UObject/Class.h"

/// Tag indicating enemy actors
inline static FName EnemyTag = FName("Enemy");

/// Action states
UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Dead,
	Patrolling,
	Chasing,
	/// Start phase of attacking
	Attacking,
	/// Actually engaged in combat i.e. swinging weapon
	Engaged,
	/// Not in any "active" state, for resetting
	NoState
};