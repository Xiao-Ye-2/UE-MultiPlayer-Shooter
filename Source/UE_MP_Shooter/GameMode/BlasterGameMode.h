// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "UE_MP_Shooter/PlayerController/MPPlayerController.h"
#include "BlasterGameMode.generated.h"

class AMPPlayerController;
class AMPCharacter;
class AMPPlayerState;

namespace MatchState
{
	extern UE_MP_SHOOTER_API const FName Cooldown;
}
/**
 * 
 */
UCLASS()
class UE_MP_SHOOTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABlasterGameMode();
	virtual void Tick(float DeltaSeconds) override;
	virtual void PlayerEliminated(AMPCharacter* EliminatedCharacter, AMPPlayerController* EliminatedPlayerController, AMPPlayerController* AttakerPlayerController);
	virtual void RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedPlayerController);
	void PlayerLeftGame(AMPPlayerState* LeavingPlayerState) const;
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;
	float LevelStaringTime = 0.f;
	

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	
private:
	float CountDownTime = 0.f;

public:
	FORCEINLINE float GetCountDownTime() const { return CountDownTime; }
};
