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
	
	void Equip(USceneComponent* SceneComponent, FName InSocketName, TObjectPtr<AActor> OwnerActor, TObjectPtr<APawn> InstigatorActor);
	void AttachMeshToComponent(USceneComponent* SceneComponent, FName InSocketName);
	bool ActorSameTagAsOwner(AActor* OtherActor, FName Tag);

	/// Play Equip sound effect
	void PlayEquipSound();

	FORCEINLINE TObjectPtr<UBoxComponent> GetCollisionBox() const { return CollisionBox; }

	/// Dynamic ignore actors array for weapon hit collision
	TArray<AActor*> CollisionIgnoreActors;

protected:
	virtual void BeginPlay() override;
	
	/// Collision box overlaps
	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/// Implement in BP, but trigger in C++ when weapon attacks
	UFUNCTION(BlueprintImplementableEvent)
	void CreateFields(const FVector& FieldLocation);

private:
	/// How much damage this weapon deals
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float Damage = 40;
	
	/// Equip sound for the weapon
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TObjectPtr<USoundBase> EquipSound;

	/// Collision box
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TObjectPtr<UBoxComponent> CollisionBox;

	/// Start for collision box tracing
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> BoxTraceStart;
	/// End for collision box tracing
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> BoxTraceEnd;
};
