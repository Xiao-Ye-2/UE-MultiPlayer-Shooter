// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;
/**
 * 
 */
UCLASS()
class UE_MP_SHOOTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(Meta = (BindWidget))
	UProgressBar* HealthBar;
	UPROPERTY(Meta = (BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(Meta = (BindWidget))
	UProgressBar* ShieldBar;
	UPROPERTY(Meta = (BindWidget))
	UTextBlock* ShieldText;

	UPROPERTY(Meta = (BindWidget))
	UTextBlock* ScoreAmount;

	UPROPERTY(Meta = (BindWidget))
	UTextBlock* DefeatsAmount;

	UPROPERTY(Meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount;

	UPROPERTY(Meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount;

	UPROPERTY(Meta = (BindWidget))
	UTextBlock* MatchCountDownText;

	UPROPERTY(Meta = (BindWidget))
	UTextBlock* GrenadesText;

	UPROPERTY(Meta = (BindWidget))
	UImage* HighPingImage;
	UPROPERTY(Meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HighPingAnimation;
};
