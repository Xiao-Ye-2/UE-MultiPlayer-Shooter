// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "UE_MP_Shooter/PlayerController/MPPlayerController.h"
#include "BlasterGameMode.generated.h"

class AMPPlayerController;
class AMPCharacter;
/**
 * 
 */
UCLASS()
class UE_MP_SHOOTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(AMPCharacter* EliminatedCharacter, AMPPlayerController* EliminatedPlayerController, AMPPlayerController* AttakerPlayerController);
	virtual void RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedPlayerController);
};
