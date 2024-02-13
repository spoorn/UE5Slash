// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Soul.h"

#include "NiagaraSystem.h"
#include "Asset/AssetMacros.h"
#include "Interfaces/PickupInterface.h"

ASoul::ASoul()
{
	// Default pickup effect and sound
	LOAD_ASSET_TO_VARIABLE(UNiagaraSystem, "/Game/Effects/Niagara/NS_SoulPickup", PickupEffect);
	LOAD_ASSET_TO_VARIABLE(USoundBase, "/Game/Audio/MetaSounds/SFX_SoulPickup", PickupSound);
}

void ASoul::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                 int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor))
	{
		PickupInterface->AddSouls(this);
		SpawnPickupSystem();
		PlayPickupSound();
		Destroy();
	}
}
