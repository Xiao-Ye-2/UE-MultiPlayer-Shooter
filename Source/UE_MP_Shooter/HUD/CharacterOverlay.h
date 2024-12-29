// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

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
	UTextBlock* ScoreAmount;
};
