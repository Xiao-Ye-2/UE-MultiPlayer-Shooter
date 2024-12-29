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
