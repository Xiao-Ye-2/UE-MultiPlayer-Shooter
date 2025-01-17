// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "UE_MP_Shooter/Interfaces/InteractWithCrosshairInterface.h"
#include "UE_MP_Shooter/MPComponents/CombatComponent.h"
#include "UE_MP_Shooter/MPTypes/ECombatStates.h"
#include "UE_MP_Shooter/MPTypes/TurningInPlace.h"
#include "MPCharacter.generated.h"

class ULagCompensationComponent;
class UBoxComponent;
class UCombatComponent;
class UBuffComponent;
class AMPPlayerState;
class FOnTimelineFloat;
class AMPPlayerController;
class UNiagaraSystem;
class UNiagaraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

UCLASS()
class UE_MP_SHOOTER_API AMPCharacter : public ACharacter, public  IInteractWithCrosshairInterface
{
	GENERATED_BODY()

public:
	AMPCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayEliminateMontage();
	void PlayThrowGrenadeMontage();
	void PlaySwapMontage();
	void UpdateHUDHealth();
	void UpdateHUDShield();
	void SpawnDefaultWeapon();
	virtual void OnRep_ReplicatedMovement() override;
	virtual void Destroyed() override;

	void Eliminate(bool bPlayerLeftGame);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminate(bool bPlayerLeftGame);

	FOnLeftGame OnLeftGame;
	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	UPROPERTY()
	AMPPlayerState* MPPlayerState;
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;
	bool bFinishedSwapping = false;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	UPROPERTY()
	TMap<FName, UBoxComponent*> HitCollisionBoxes;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();
protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Value);
	void Turn(float Value);

	void EquipButtonPressed();
	void CrouchButtonPressed();
	void ReloadButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	float CalculateSpeed() const;
	void CalculateAO_Pitch();
	void AimOffset(float DeltaTime);
	void RotateInPlace(float DeltaTime);
	void SimProxiesTurn();
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	void GrenadeButtonPressed();
	void PlayHitReactMontage();

	void PollAndInitialize();
	UFUNCTION()
	void ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);
	
	/**
	 * Hit Boxes
	 */
	UPROPERTY(EditAnywhere)
	UBoxComponent* head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;
	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* backpack;
	UPROPERTY(EditAnywhere)
	UBoxComponent* blanket;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	UPROPERTY()
	AMPPlayerController* MPPlayerController;

	UPROPERTY(VisibleAnywhere, blueprintReadOnly, meta = (AllowPrivateAccess = true))
	UCombatComponent* CombatComponent;
	UPROPERTY(VisibleAnywhere, blueprintReadOnly, meta = (AllowPrivateAccess = true))
	UBuffComponent* BuffComponent;
	UPROPERTY(VisibleAnywhere, blueprintReadOnly, meta = (AllowPrivateAccess = true))
	ULagCompensationComponent* LagCompensationComponent;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon) const;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	float AO_Yaw;
	float Interp_AO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);
	
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* FireWeaponMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGrenadeMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* SwapMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	float CameraThreshold;
	void HideCameraIfCharacterClose() const;

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;

	/**
	 * Health
	 */
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;
	UFUNCTION()
	void OnRep_Health(float LastHealth);

	/**
	 * Shield
	 */
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")
	float Shield = 100.f;
	UFUNCTION()
	void OnRep_Shield(float LastShield);

	/**
	 * Dissolve Effects and Elimination
	 */
	bool bEliminated = false;

	FTimerHandle EliminatedTimer;
	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
	float EliminateDelay = 3.f;
	void EliminateTimerFinished();
	void WeaponHandleWhenEliminated(AWeapon* Weapon);
	
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;
	FOnTimelineFloat DissolveTrack;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	UPROPERTY(VisibleAnywhere, Category = "Elimination")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;
	UPROPERTY(EditAnywhere, Category = "Elimination")
	UMaterialInstance* DissolveMaterialInstance;

	UPROPERTY(VisibleAnywhere, Category = "Elimination")
	UParticleSystemComponent* EliminationEffectComponent;
	UPROPERTY(EditAnywhere, Category = "Elimination")
	UParticleSystem* EliminationEffect;
	UPROPERTY(EditAnywhere, Category = "Elimination")
	USoundCue* EliminationEffectSound;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* CrownSystem;
	UPROPERTY()
	UNiagaraComponent* CrownComponent;

	bool bLeftGame = false;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;
	
public:	
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped() const;
	bool IsAiming() const;
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AWeapon* GetEquippedWeapon() const;
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsEliminated() const { return bEliminated; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(float NewHealth) { Health = NewHealth; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE void SetShield(float NewShield) { Shield = NewShield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent; }
	FORCEINLINE UBuffComponent* GetBuffComponent() const { return BuffComponent; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE bool IsLocallyReloading() const { return CombatComponent == nullptr ? false : CombatComponent->bLocallyReloading; }
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComponent() const { return LagCompensationComponent; }
	ECombatStates GetCombatState() const;
};
