// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item.h"

#include "Character/SlashCharacter.h"
#include "Components/SphereComponent.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	RootComponent = ItemMesh;

	SphereComponent = CreateDefaultSubobject<USphereComponent>("ItemSphereComponent");
	SphereComponent->SetupAttachment(GetRootComponent());
	SphereComponent->SetSphereRadius(100);
}

void AItem::BeginPlay()
{
	Super::BeginPlay();

	// Bind our callback to SphereComponent's OnComponentBeginOverlap event
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereBeginOverlap);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);
}

float AItem::TransformedSin()
{
	return FMath::Sin(RunningTime * TimeConstant) * Amplitude;
}

float AItem::TransformedCos()
{
	return FMath::Cos(RunningTime * TimeConstant) * Amplitude;
}

void AItem::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (TObjectPtr<ASlashCharacter> SlashCharacter = Cast<ASlashCharacter>(OtherActor); SlashCharacter)
	{
		SlashCharacter->SetOverlappingItem(this);
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// Only remove overlapping item from character if it is the current one
	if (TObjectPtr<ASlashCharacter> SlashCharacter = Cast<ASlashCharacter>(OtherActor); SlashCharacter && SlashCharacter->GetOverlappingItem() == this)
	{
		SlashCharacter->SetOverlappingItem(nullptr);
	}
}

void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;
	//
	// FVector Location = GetActorLocation();
	// DRAW_SPHERE_SINGLE_FRAME(Location);
	//
	// FVector Forward = GetActorForwardVector();
	// DRAW_VECTOR_SINGLE_FRAME(Location, Location + Forward * 100.0f);

	// Float item when hovering
	if (ItemState == EItemState::Hovering)
	{
		AddActorWorldOffset(FVector(0, 0, TransformedSin()));
	}
}

