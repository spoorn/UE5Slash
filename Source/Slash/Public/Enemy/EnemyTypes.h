#pragma once

/// Action states
UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Patrolling,
	Chasing,
	Attacking
};