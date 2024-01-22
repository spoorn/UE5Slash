// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "HealthBarComponent.generated.h"

/**
 * 
 */
UCLASS()
class SLASH_API UHealthBarComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	/// Set health percent for this component, which will update the HealthBar widget
	void SetHealthPercent(float Percent);

private:
	// Health bar widget
	TObjectPtr<class UHealthBar> HealthBarWidget;
};
