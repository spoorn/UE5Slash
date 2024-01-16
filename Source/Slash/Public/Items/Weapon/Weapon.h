// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Weapon.generated.h"

class UBoxComponent;
/**
 * Base Weapon class
 *
 * Note: PrioritizeCategories doesn't seem to work: https://forums.unrealengine.com/t/reorder-variable-categories-in-class-defaults/66561/78
 */
UCLASS(meta = (PrioritizeCategories ="Physics Input Collision Lighting"))
class SLASH_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	AWeapon();
	
	void Equip(USceneComponent* SceneComponent, FName InSocketName);
	void AttachMeshToComponent(USceneComponent* SceneComponent, FName InSocketName);

	/// Play Equip sound effect
	void PlayEquipSound();

	FORCEINLINE TObjectPtr<UBoxComponent> GetCollisionBox() const { return CollisionBox; }

protected:
	virtual void BeginPlay() override;
	
	// Pickup overlaps
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	// Collision box overlaps
	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	/// Equip sound for the weapon
	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	TObjectPtr<USoundBase> EquipSound;

	/// Collision box
	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	TObjectPtr<UBoxComponent> CollisionBox;

	/// Start for collision box tracing
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> BoxTraceStart;
	/// End for collision box tracing
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> BoxTraceEnd;
};
