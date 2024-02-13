// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SlashHUD.generated.h"

class USlashOverlay;
/**
 * 
 */
UCLASS()
class SLASH_API ASlashHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	FORCEINLINE USlashOverlay* GetSlashOverlay() const { return SlashOverlay; };

private:
	UPROPERTY(EditDefaultsOnly, Category = Slash)
	TSubclassOf<USlashOverlay> SlashOverlayClass;

	UPROPERTY()
	TObjectPtr<USlashOverlay> SlashOverlay;
};
