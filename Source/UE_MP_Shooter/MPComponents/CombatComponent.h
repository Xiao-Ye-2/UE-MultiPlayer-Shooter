// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UE_MP_Shooter/HUD/MPHUD.h"
#include "UE_MP_Shooter/MPTypes/ECombatStates.h"
#include "UE_MP_Shooter/Weapon/WeaponTypes.h"
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
	void Reload();
	void UpdateAmmoAfterReload();
	void FireButtonPressed(bool bPressed);
	UFUNCTION(BlueprintCallable)
	void FinishReloading();
protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();
	void Fire();
	bool CanFire() const;

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerReload();
	void HandleReload();
	int32 AmountToReload();

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);
	
	void SetHUDCrosshairs(float DeltaTime);
private:
	UPROPERTY()
	AMPCharacter* Character;
	UPROPERTY()
	AMPPlayerController* PlayerController;
	UPROPERTY()
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

	/**
	 * Ammos
	 */
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;
	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;
	UPROPERTY(EditAnywhere)
	int32 InitialCarriedAmmo = 30;
	void InitializeCarriedAmmo();
	void UpdateCarriedAmmoHUD();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatStates CombatState = ECombatStates::ECS_Unoccupied;
	UFUNCTION()
	void OnRep_CombatState();
public:	

		
};

