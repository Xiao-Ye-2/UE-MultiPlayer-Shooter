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
enum class EWeaponStates : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped State"),
	EWS_Dropped UMETA(DisplayName = "Dropped State"),

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
	virtual void OnRep_Owner() override;
	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);
	void Drop();
	void SetHUDWeaponAmmo();
	void AddAmmo(int32 Amount);

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

protected:
	virtual void BeginPlay() override;

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
	
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;

	UFUNCTION()
	void OnRep_Ammo();
	void SpendRound();
	
	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	UPROPERTY()
	AMPCharacter* OwnerCharacter;
	UPROPERTY()
	AMPPlayerController* OwnerController;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
	
public:
	void SetWeaponState(EWeaponStates State);
	void OnWeaponStateSet();
	FORCEINLINE USphereComponent* GetAreaSphere() { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE bool IsEmpty() const { return Ammo <= 0; }
};
