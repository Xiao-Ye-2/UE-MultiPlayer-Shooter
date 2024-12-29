// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "UE_MP_Shooter/Character/MPCharacter.h"
#include "UE_MP_Shooter/HUD/CharacterOverlay.h"
#include "UE_MP_Shooter/HUD/MPHUD.h"

void AMPPlayerController::BeginPlay()
{
	Super::BeginPlay();

	MPHUD = Cast<AMPHUD>(GetHUD());
}

void AMPPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AMPCharacter* MPCharacter = Cast<AMPCharacter>(InPawn);
	if (MPCharacter)
	{
		SetHUDHealth(MPCharacter->GetHealth(), MPCharacter->GetMaxHealth());
	}
}

void AMPPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
	if (MPHUD && MPHUD->CharacterOverlay && MPHUD->CharacterOverlay->HealthBar && MPHUD->CharacterOverlay->HealthText)
	{
		const float HealthPercent = Health / MaxHealth;
		MPHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		MPHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void AMPPlayerController::SetHUDScore(float Score)
{
	MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
	if (MPHUD && MPHUD->CharacterOverlay && MPHUD->CharacterOverlay->ScoreAmount)
	{
		MPHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(FString::FromInt(FMath::CeilToInt(Score))));
	}
}

void AMPPlayerController::SetHUDDefeats(int32 Defeats)
{
	MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
	if (MPHUD && MPHUD->CharacterOverlay && MPHUD->CharacterOverlay->DefeatsAmount)
	{
		MPHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(FString::FromInt(Defeats)));
	}
}

void AMPPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
	if (MPHUD && MPHUD->CharacterOverlay && MPHUD->CharacterOverlay->WeaponAmmoAmount)
	{
		MPHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(FString::FromInt(Ammo)));
	}
}

void AMPPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
	if (MPHUD && MPHUD->CharacterOverlay && MPHUD->CharacterOverlay->CarriedAmmoAmount)
	{
		MPHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(FString::FromInt(Ammo)));
	}
}
