// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item.h"

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
	if (const FString OtherActorName = OtherActor->GetName(); GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 30, FColor::Red, FString::Printf(TEXT("Begin Overlap: %s"), *OtherActorName));
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (const FString OtherActorName = OtherActor->GetName(); GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 30, FColor::Red, FString::Printf(TEXT("End Overlap: %s"), *OtherActorName));
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
}

