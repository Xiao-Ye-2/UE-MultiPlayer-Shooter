// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


class AMPCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE_MP_SHOOTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend AMPCharacter;

	void Heal(float HealAmount, float HealingTime);
	void BuffSpeed(float WalkBuffSpeed, float CrouchBuffSpeed, float BuffTime);
	void SetInitialSpeed(float WalkSpeed, float CrouchSpeed, float JumpSpeed);
	void BuffJump(float JumpSpeed, float BuffTime);

protected:
	virtual void BeginPlay() override;
	void HealthRampUp(float DeltaTime);

private:
	UPROPERTY()
	AMPCharacter* Character;

	bool bHealing = false;
	float HealingRate = 0;
	float AmountToHeal = 0.f;

	FTimerHandle SpeedBuffTimer;
	void ResetSpeed();
	float InitialWalkSpeed;
	float InitialCrouchSpeed;
	UFUNCTION(NetMulticast, reliable)
	void MulticastSpeedBuff(float WalkSpeed, float CrouchSpeed);

	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpSpeed;
	UFUNCTION(NetMulticast, reliable)
	void MulticastJumpBuff(float JumpSpeed);

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
