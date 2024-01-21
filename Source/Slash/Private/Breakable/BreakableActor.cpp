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

	TreasureClasses.Add(ATreasure::StaticClass());

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("BreakableCapsuleComponent");
	CapsuleComponent->SetupAttachment(GetRootComponent());

	// Chaos Physics for geometry collections default to Dynamic, else it won't fracture properly, so we
	// don't want the geometry collection itself colliding with objects, causing it to move, so we ignore here
	// and instead add a capsule component for the actual collision.  The only thing geometry collection should block
	// are WorldStatic objects, such as the floor to prevent it from falling through
	GeometryCollectionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	GeometryCollectionComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GeometryCollectionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	// Prevent Geometry Collection from colliding with Capsule Component
	CapsuleComponent->SetCollisionObjectType(ECC_PhysicsBody);
	// Query Only will physically block character, but not ran in the physics simulation
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CapsuleComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

void ABreakableActor::GetHit_Implementation(const FVector& ImpactPoint)
{
	if (bBroken) return;  // Prevent this from being called multiple times due to multiple fractured meshes
	if (UWorld* World = GetWorld(); World && TreasureClasses.Num() > 0)
	{
		FVector Location = GetActorLocation();
		Location.Z += 75;
		const int32 Selection = FMath::RandRange(0, TreasureClasses.Num() - 1);
		World->SpawnActor<ATreasure>(TreasureClasses[Selection], Location, GetActorRotation());
	}
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	bBroken = true;
}

void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

