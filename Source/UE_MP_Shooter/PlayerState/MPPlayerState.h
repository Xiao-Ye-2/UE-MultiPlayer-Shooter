// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MPPlayerState.generated.h"

class AMPPlayerController;
class AMPCharacter;
/**
 * 
 */
UCLASS()
class UE_MP_SHOOTER_API AMPPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void OnRep_Score() override;
	void AddToScore(float ScoreAmount);

private:
	AMPCharacter* Character;
	AMPPlayerController* Controller;

	void UpdateScoreHUD();
};
