// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UE_MP_Shooter/HUD/MPHUD.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f;

class AMPPlayerController;
class AMPHUD;
class AWeapon;
class AMPCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE_MP_SHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend AMPCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(AWeapon* WeaponToEquip);

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();
	void Fire();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);
	
	void SetHUDCrosshairs(float DeltaTime);
private:
	AMPCharacter* Character;
	AMPPlayerController* PlayerController;
	AMPHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	/**
	 *  HUD and Crosshairs
	 */
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;
	FVector HitTarget;
	FHUDPackage HUDPackage;

	/**
	 *  FOV and Aim
	 */
	float DefaultFOV;
	float CurrentFOV;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	float UnZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/**
	 * Automatic Fire
	 */
	FTimerHandle FireTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bCanFire = true;
	void StartFireTimer();
	void FireTimerFinished();
public:	

		
};
