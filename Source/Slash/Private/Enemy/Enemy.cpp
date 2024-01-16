// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

#include "Components/CapsuleComponent.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// Enemy mesh should be WorldDynamic to have collision with player weapons
	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	// Block visibility channel for hit traces
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// Set default mesh
	if (const ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMesh(TEXT("/Game/Mixamo/Paladin/SKM_Paladin"))
		; SkeletalMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMeshAsset(SkeletalMesh.Object);
		// Match default mesh with capsule
		GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));
		GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	}
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

