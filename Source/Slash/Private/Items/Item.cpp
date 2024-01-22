// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item.h"

#include "Character/SlashCharacter.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "Asset/AssetMacros.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;

	// Blanket root component so other components can be transformed
	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	ItemMesh->SetupAttachment(GetRootComponent());

	SphereComponent = CreateDefaultSubobject<USphereComponent>("ItemSphereComponent");
	SphereComponent->SetupAttachment(ItemMesh);
	SphereComponent->SetSphereRadius(100);

	GlowParticles = CreateDefaultSubobject<UNiagaraComponent>("GlowParticles");
	GlowParticles->SetupAttachment(ItemMesh);
	LOAD_ASSET_TO_CALLBACK(UNiagaraSystem, "/Game/Effects/Niagara/NS_Embers.NS_Embers", GlowParticles->SetAsset);
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
	if (TObjectPtr<ASlashCharacter> SlashCharacter = Cast<ASlashCharacter>(OtherActor))
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
		AddActorWorldRotation(FRotator(0, DeltaTime * -45, 0));
	}
}

