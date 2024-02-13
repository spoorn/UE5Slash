// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SlashOverlay.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void USlashOverlay::SetHealthPercent(float Percent)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(Percent);
	}
}

void USlashOverlay::SetStaminaPercent(float Percent)
{
	if (StaminaBar)
	{
		StaminaBar->SetPercent(Percent);
	}
}

void USlashOverlay::SetGold(int32 Gold)
{
	if (GoldCount)
	{
		GoldCount->SetText(FText::AsNumber(Gold));
	}
}

void USlashOverlay::SetSouls(int32 Souls)
{
	if (SoulsCount)
	{
		SoulsCount->SetText(FText::AsNumber(Souls));
	}
}
