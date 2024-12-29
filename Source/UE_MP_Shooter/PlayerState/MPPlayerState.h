// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MPPlayerState.generated.h"

class AMPPlayerController;
class AMPCharacter;
class FLifetimeProperty;
/**
 * 
 */
UCLASS()
class UE_MP_SHOOTER_API AMPPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Score() override;
	UFUNCTION()
	virtual void OnRep_Defeats();
	bool IsControllerValid();
	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount);

private:
	UPROPERTY()
	AMPCharacter* Character;
	UPROPERTY()
	AMPPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;

	void UpdateScoreHUD();
	void UpdateDefeatsHUD();
};
