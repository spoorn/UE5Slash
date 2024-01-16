// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapon/Weapon.h"

#include "Asset/AssetMacros.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

AWeapon::AWeapon()
{
	//LOAD_ASSET_TO_VARIABLE(USoundBase, "/Game/Audio/MetaSounds/SFX_Shink'", EquipSound);
}

void AWeapon::PlayEquipSound()
{
	if (EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquipSound, GetActorLocation());
	}
}

void AWeapon::Equip(USceneComponent* SceneComponent, FName InSocketName)
{
	if (SceneComponent)
	{
		AttachMeshToComponent(SceneComponent, InSocketName);
		ItemState = EItemState::Equipped;
		PlayEquipSound();
		if (SphereComponent)
		{
			// Disable collision for the sphere so we don't get overlapping item anymore
			SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AWeapon::AttachMeshToComponent(USceneComponent* SceneComponent, FName InSocketName)
{
	ItemMesh->AttachToComponent(SceneComponent, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false), InSocketName);
}

void AWeapon::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                   int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	Super::OnSphereEndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}
