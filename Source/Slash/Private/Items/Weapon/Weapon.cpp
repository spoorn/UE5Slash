// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapon/Weapon.h"

#include "NiagaraComponent.h"
#include "Asset/AssetMacros.h"
#include "Character/CharacterTypes.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Enemy/EnemyTypes.h"
#include "Interfaces/HitInterface.h"
#include "Kismet/GameplayStatics.h"

AWeapon::AWeapon()
{
	// Default equip sound
	LOAD_ASSET_TO_VARIABLE(USoundBase, "/Game/Audio/MetaSounds/SFX_Shink", EquipSound);

	// Use collision box instead
	ItemMesh->SetCollisionProfileName(FName("NoCollision"));

	CollisionBox = CreateDefaultSubobject<UBoxComponent>("WeaponCollisionBox");
	CollisionBox->SetupAttachment(ItemMesh);
	
	// Default for swords
	CollisionBox->InitBoxExtent(FVector(3, 2.2, 40));
	CollisionBox->SetRelativeLocation(FVector(0, 0, 12.4));
	
	// Default to no collision at the start
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	//CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	// Positions will be set in BP
	BoxTraceStart = CreateDefaultSubobject<USceneComponent>("BoxTraceStart");
	BoxTraceStart->SetupAttachment(ItemMesh);
	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>("BoxTraceEnd");
	BoxTraceEnd->SetupAttachment(ItemMesh);
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

void AWeapon::Equip(USceneComponent* SceneComponent, FName InSocketName, TObjectPtr<AActor> OwnerActor, TObjectPtr<APawn> InstigatorActor)
{
	SetOwner(OwnerActor);
	SetInstigator(InstigatorActor);
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
		if (GlowParticles)
		{
			GlowParticles->Deactivate();
		}
	}
}

void AWeapon::AttachMeshToComponent(USceneComponent* SceneComponent, FName InSocketName)
{
	ItemMesh->AttachToComponent(SceneComponent, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false), InSocketName);
}

bool AWeapon::ActorSameTagAsOwner(AActor* OtherActor, FName Tag)
{
	return GetOwner()->ActorHasTag(Tag) && OtherActor->ActorHasTag(Tag);
}

void AWeapon::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Enemies cannot hurt other enemies
	// SlashCharacters cannot hurt other SlashCharacters
	if (ActorSameTagAsOwner(OtherActor, EnemyTag) || ActorSameTagAsOwner(OtherActor, SlashCharacterTag)) return;
	CollisionIgnoreActors.Add(GetOwner());
	// Ignore the owner of this weapon, or if we already hit the other actor during a single attack
	if (!CollisionIgnoreActors.Contains(OtherActor))
	{
		const FVector Start = BoxTraceStart->GetComponentLocation();
		const FVector End = BoxTraceEnd->GetComponentLocation();
		const FVector BoxHalfSize = FVector(5 * CollisionBox->GetComponentScale().X, 5 * CollisionBox->GetComponentScale().Y, 5);
		FHitResult HitResult;
		// Note: trace type query is for custom traces, which we aren't using here so just pick any
		UKismetSystemLibrary::BoxTraceSingle(this, Start, End, BoxHalfSize, BoxTraceStart->GetComponentRotation(),
			ETraceTypeQuery::TraceTypeQuery1, false, CollisionIgnoreActors, EDrawDebugTrace::ForDuration, HitResult, true);

		if (AActor* HitActor = HitResult.GetActor();
			HitActor
			&& !ActorSameTagAsOwner(HitActor, EnemyTag)
			&& !ActorSameTagAsOwner(HitActor, SlashCharacterTag)
			&& HitActor != GetOwner()
			&& !CollisionIgnoreActors.Contains(HitActor))
		{
			UE_LOG(LogTemp, Warning, TEXT("Owner: %s, Hit: %s"), *GetOwner()->GetName(), *HitActor->GetName());
			
			UGameplayStatics::ApplyDamage(HitActor, Damage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
			
			if (IHitInterface* HitInterface = Cast<IHitInterface>(HitActor))
			{
				HitInterface->Execute_GetHit(HitActor, HitResult.ImpactPoint);
			}
			// Hitting terrain generates a physics field too
			CreateFields(HitResult.ImpactPoint);
			CollisionIgnoreActors.AddUnique(HitActor);
		}
	}
}
