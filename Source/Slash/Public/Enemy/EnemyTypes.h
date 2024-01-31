#pragma once

#include "UObject/Class.h"

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
	Engaged
};