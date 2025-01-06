// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class AMPPlayerController;
class AMPCharacter;
class ACasing;
class UWidgetComponent;
class UAnimationAsset;

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),
	
	EFT_MAX UMETA(DisplayName = "Max")
};
UENUM(BlueprintType)
enum class EWeaponStates : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped State"),
	EWS_Dropped UMETA(DisplayName = "Dropped State"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary State"),
	
	EWS_MAX UMETA(DisplayName = "Default MAX"),
};

UCLASS()
class UE_MP_SHOOTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Fire(const FVector& HitTarget);
	virtual void OnRep_Owner() override;
	void SetDefaultCustomDepthEnabled();
	void ShowPickupWidget(bool bShowWidget);
	void Drop();
	void SetHUDWeaponAmmo();
	void AddAmmo(int32 Amount);
	void EnabledCustomDepth(bool bEnabled) const;
	FVector TraceEndWithScatter(const FVector& HitTarget) const;

	/**
	 * Textures for the weapon crosshairs
	 */
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairCenter;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairLeft;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairRight;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairTop;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairBottom;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float FireDelay = .15f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere, Category = "Combat")
	USoundCue* EquipSound;

	bool bDestroyWeaponWhenDrop = false;
	UPROPERTY(EditAnywhere, Category = "Combat")
	EFireType FireType = EFireType::EFT_HitScan;
protected:
	virtual void BeginPlay() override;
	UPROPERTY()
	AMPCharacter* OwnerCharacter;
	UPROPERTY()
	AMPPlayerController* OwnerController;
	
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;
	UPROPERTY(Replicated, EditAnywhere)
	bool bUseServerSideRewind = false;
	UFUNCTION()
	void OnPingHigh(bool bPingHigh);
private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponStates WeaponState;
	UFUNCTION()
	void OnRep_WeaponState();
	void OnEquipped();
	void OnEquippedSecondary();
	void OnDropped();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UWidgetComponent* PickupWidget;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	UAnimationAsset* FireAnimation;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<ACasing> CasingClass;

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	/**
	 * Ammo Settings
	 */
	
	UPROPERTY(EditAnywhere)
	int32 Ammo;
	void SpendRound();
	UFUNCTION(Client, reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);
	UFUNCTION(Client, reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	int32 Sequence = 0; // Number of unprocessed server request for Ammo.
	
	UPROPERTY(EditAnywhere)
	int32 MagCapacity;
	
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

public:
	void SetWeaponState(EWeaponStates State);
	FORCEINLINE USphereComponent* GetAreaSphere() { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE bool IsEmpty() const { return Ammo <= 0; }
	FORCEINLINE bool IsFull() const { return Ammo >= MagCapacity; }
	FORCEINLINE bool IsUsingScatter() const { return bUseScatter; }
	FORCEINLINE float GetDamage() const { return Damage; }
};


