// Fill out your copyright notice in the Description page of Project Settings.


#include "Breakable/BreakableActor.h"

#include "Components/CapsuleComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Items/Treasure/Treasure.h"

ABreakableActor::ABreakableActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	GeometryCollectionComponent = CreateDefaultSubobject<UGeometryCollectionComponent>("GeometryCollection");
	SetRootComponent(GeometryCollectionComponent);
	GeometryCollectionComponent->SetGenerateOverlapEvents(true);
	GeometryCollectionComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GeometryCollectionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	TreasureClass = ATreasure::StaticClass();

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("BreakableCapsuleComponent");
	CapsuleComponent->SetupAttachment(GetRootComponent());
	CapsuleComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

void ABreakableActor::GetHit_Implementation(const FVector& ImpactPoint)
{
	if (bBroken) return;  // Prevent this from being called multiple times due to multiple fractured meshes
	if (UWorld* World = GetWorld(); World && TreasureClass)
	{
		FVector Location = GetActorLocation();
		Location.Z += 75;
		World->SpawnActor<ATreasure>(TreasureClass, Location, GetActorRotation());
	}
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	bBroken = true;
}

void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

