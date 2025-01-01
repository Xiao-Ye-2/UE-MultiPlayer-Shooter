// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileBullet.h"
#include "ProjectileGrenade.generated.h"

/**
 * 
 */
UCLASS()
class UE_MP_SHOOTER_API AProjectileGrenade : public AProjectileBullet
{
	GENERATED_BODY()

public:
	AProjectileGrenade();
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

private:
	UPROPERTY(EditAnywhere)
	USoundCue* BounceSound;
};
