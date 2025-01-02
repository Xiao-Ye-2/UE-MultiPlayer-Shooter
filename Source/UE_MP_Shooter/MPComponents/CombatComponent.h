// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UE_MP_Shooter/HUD/MPHUD.h"
#include "UE_MP_Shooter/MPTypes/ECombatStates.h"
#include "UE_MP_Shooter/Weapon/WeaponTypes.h"
#include "CombatComponent.generated.h"

class AProjectile;
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
	void SwapWeapons();
	void Reload();
	void FireButtonPressed(bool bPressed);
	
	UFUNCTION(BlueprintCallable)
	void FinishReloading();
	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();
	void JumpToShotgunEnd() const;

	void PickupAmmo(EWeaponType WeaponType, int32 Amount);
protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_PrimaryWeapon();
	UFUNCTION()
	void OnRep_SecondaryWeapon();
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
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
	
	void ThrowGrenade();
	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();
	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();
	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();
	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> GrenadeProjectileClass;

	void DropEquippedWeapon() const;
	void AttachActorToRightHand(AActor* ActorToAttach) const;
	void AttachActorToLeftHand(AActor* ActorToAttach) const;
	void AttachActorToBackpack(AActor* ActorToAttach) const;
	void AttachActorToSocket(AActor* ActorToAttach, FName SocketName) const;
	void PlayWeaponEquipSound(AWeapon* WeaponToEquip) const;
	void ReloadIfWeaponEmpty();
private:
	UPROPERTY()
	AMPCharacter* Character;
	UPROPERTY()
	AMPPlayerController* PlayerController;
	UPROPERTY()
	AMPHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_PrimaryWeapon)
	AWeapon* EquippedWeapon;
	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

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
	int32 InitialRifleCarriedAmmo = 60;
	UPROPERTY(EditAnywhere)
	int32 InitialRocketCarriedAmmo = 4;
	UPROPERTY(EditAnywhere)
	int32 InitialPistolCarriedAmmo = 30;
	UPROPERTY(EditAnywhere)
	int32 InitialSMGCarriedAmmo = 40;
	UPROPERTY(EditAnywhere)
	int32 InitialShotgunCarriedAmmo = 10;
	UPROPERTY(EditAnywhere)
	int32 InitialSniperCarriedAmmo = 2;
	UPROPERTY(EditAnywhere)
	int32 InitialGrenadeLauncherCarriedAmmo = 4;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxCarriedAmmo = 400;

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 2;
	UFUNCTION()
	void OnRep_Grenades();
	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 4;
	void InitializeCarriedAmmo();
	void UpdateCarriedAmmoHUD();
	void UpdateCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatStates CombatState = ECombatStates::ECS_Unoccupied;
	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoAfterReload();
	void UpdateShotgunAmmoAfterReload();
	void ShowAttachedGrenade(bool bShowGrenade) const;
public:	
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	FORCEINLINE bool ShouldSwapWeapons() const { return EquippedWeapon != nullptr && SecondaryWeapon != nullptr; }
};

