// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/HitInterface.h"
#include "BreakableActor.generated.h"

UCLASS()
class SLASH_API ABreakableActor : public AActor, public IHitInterface
{
	GENERATED_BODY()
	
public:	
	ABreakableActor();

	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	/// Called when the geometry collection breaks due to chaos physics
	UFUNCTION()
	void HandleOnChaosBreakEvent(const FChaosBreakEvent& BreakEvent);

protected:
	virtual void BeginPlay() override;

	/// Fractured geometry collection
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UGeometryCollectionComponent> GeometryCollectionComponent;

	/// Capsule component for collision with Pawn actors only, as the Breakable Actor itself has no
	/// collision with Pawns so fractured pieces don't collide
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UCapsuleComponent> CapsuleComponent;

private:
	bool bBroken = false;
	
	/// Treasure class to spawn
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class ATreasure>> TreasureClasses;
};
