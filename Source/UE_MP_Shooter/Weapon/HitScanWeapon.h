// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class UE_MP_SHOOTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;

protected:
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);
	void WeaponTraceHit(FHitResult& Hit, const FVector& TraceStart, const FVector& HitTarget, float HitSoundVolumeMultiplier = 1.f, float HitSoundPitchMultiplier = 1.f);

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticleSystem;

	UPROPERTY(EditAnywhere)
	USoundCue* HitSound;
	
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

private:
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticleSystem;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlashParticleSystem;
	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;
	
};
