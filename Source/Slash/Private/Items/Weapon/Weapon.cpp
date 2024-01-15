// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapon/Weapon.h"

void AWeapon::Equip(USceneComponent* SceneComponent, FName InSocketName)
{
	if (SceneComponent)
	{
		ItemMesh->AttachToComponent(SceneComponent, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false), InSocketName);
		ItemState = EItemState::Equipped;
	}
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
