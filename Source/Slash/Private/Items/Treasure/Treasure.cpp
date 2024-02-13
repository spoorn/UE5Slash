// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Treasure/Treasure.h"

#include "Asset/AssetMacros.h"
#include "Components/SphereComponent.h"
#include "Interfaces/PickupInterface.h"

ATreasure::ATreasure()
{
	LOAD_ASSET_TO_VARIABLE(USoundBase, "/Game/Audio/MetaSounds/SFX_Coins", PickupSound);
	// Set default mesh
	LOAD_ASSET_TO_CALLBACK(UStaticMesh, "/Game/AncientTreasures/Meshes/SM_Chalice_01a", ItemMesh->SetStaticMesh);
	// Smaller radius for treasure
	SphereComponent->SetSphereRadius(50);
}

void ATreasure::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                     int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor))
	{
		PickupInterface->AddGold(this);
		PlayPickupSound();
		// Despawn
		Destroy();
	}
}
