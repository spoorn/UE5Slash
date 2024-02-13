// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PickupInterface.generated.h"

class ATreasure;
class ASoul;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPickupInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SLASH_API IPickupInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	FORCEINLINE virtual void SetOverlappingItem(class AItem* Item) = 0;
	FORCEINLINE virtual AItem* GetOverlappingItem() = 0;
	virtual void AddSouls(ASoul* Soul) = 0;
	virtual void AddGold(ATreasure* Treasure) = 0;
};
