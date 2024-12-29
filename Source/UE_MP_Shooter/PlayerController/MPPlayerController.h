// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MPPlayerController.generated.h"

class AMPHUD;
/**
 * 
 */
UCLASS()
class UE_MP_SHOOTER_API AMPPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void virtual OnPossess(APawn* InPawn) override;

protected:
	virtual void BeginPlay() override;

private:
	AMPHUD* MPHUD;
};
