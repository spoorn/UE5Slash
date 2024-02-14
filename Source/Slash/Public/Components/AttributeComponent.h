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
	/// Callback to use stamina
	void UseStamina(float Amount);
	/// Get percentage of health left
	float GetStaminaPercent();
	/// Regenerate stamina
	void RegenStamina(float DeltaTime);

	/// Whether entity is alive based on health and max health
	bool IsAlive();

	void AddGold(int32 Amount);
	void AddSouls(int32 Amount);

	FORCEINLINE int32 GetGold() const { return Gold; }
	FORCEINLINE int32 GetSouls() const { return Souls; }
	FORCEINLINE float GetDodgeCost() const { return DodgeCost; }
	FORCEINLINE float GetStamina() const { return Stamina; }
	FORCEINLINE bool CanDodge() const { return Stamina >= DodgeCost; }

protected:
	virtual void BeginPlay() override;

private:
	/// Current health
	UPROPERTY(EditAnywhere, Category = Attributes)
	float Health = 100;

	/// Default max health
	UPROPERTY(EditAnywhere, Category = Attributes)
	float MaxHealth = 100;

	/// Current stamina
	UPROPERTY(EditAnywhere, Category = Attributes)
	float Stamina = 100;

	/// Default max stamina
	UPROPERTY(EditAnywhere, Category = Attributes)
	float MaxStamina = 100;

	/// Gold count
	UPROPERTY(VisibleAnywhere, Category = Attributes)
	int32 Gold = 0;

	/// Souls count
	UPROPERTY(VisibleAnywhere, Category = Attributes)
	int32 Souls = 0;

	/// Cost of stamina for Dodge action
	UPROPERTY(VisibleAnywhere, Category = Attributes)
	float DodgeCost = 14;

	/// Regeneration rate for stamina per second
	UPROPERTY(VisibleAnywhere, Category = Attributes)
	float StaminaRegenRate = 2;
};
