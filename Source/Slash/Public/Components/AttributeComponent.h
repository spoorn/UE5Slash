// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SLASH_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttributeComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/// Callback to apply damage
	void ReceiveDamage(float Damage);
	/// Get percentage of health left
	float GetHealthPercent();

	/// Whether entity is alive based on health and max health
	bool IsAlive();

protected:
	virtual void BeginPlay() override;

private:
	/// Current health
	UPROPERTY(EditAnywhere, Category = Attributes)
	float Health;

	/// Default max health
	UPROPERTY(EditAnywhere, Category = Attributes)
	float MaxHealth;
		
};
