// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapon/Weapon.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

AWeapon::AWeapon()
{
	// TODO: doesn't work with Metasound sources
	//LOAD_ASSET_TO_VARIABLE(USoundBase, "/Game/Audio/MetaSounds/SFX_Shink'", EquipSound);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>("WeaponCollisionBox");
	CollisionBox->SetupAttachment(GetRootComponent());
	CollisionBox->InitBoxExtent(FVector(3, 2.2, 40));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	// Positions will be set in BP
	BoxTraceStart = CreateDefaultSubobject<USceneComponent>("BoxTraceStart");
	BoxTraceStart->SetupAttachment(GetRootComponent());
	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>("BoxTraceEnd");
	BoxTraceEnd->SetupAttachment(GetRootComponent());
}

void AWeapon::PlayEquipSound()
{
	if (EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquipSound, GetActorLocation());
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxBeginOverlap);
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

void AWeapon::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Ignore the owner of this weapon
	if (OtherActor != GetOwner())
	{
		const FVector Start = BoxTraceStart->GetComponentLocation();
		const FVector End = BoxTraceEnd->GetComponentLocation();
		const FVector BoxHalfSize = FVector(5, 5, 5);
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(this);
		FHitResult HitResult;
		// Note: trace type query is for custom traces, which we aren't using here so just pick any
		UKismetSystemLibrary::BoxTraceSingle(this, Start, End, BoxHalfSize, BoxTraceStart->GetComponentRotation(),
			ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForDuration, HitResult, true);
	}
}
