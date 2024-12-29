// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UE_MP_Shooter/Character/MPCharacter.h"
#include "UE_MP_Shooter/HUD/MPHUD.h"
#include "UE_MP_Shooter/PlayerController/MPPlayerController.h"
#include "UE_MP_Shooter/Weapon/Weapon.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 350.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character == nullptr) return;
	
	Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	if (Character->GetFollowCamera())
	{
		DefaultFOV = Character->GetFollowCamera()->FieldOfView;
		CurrentFOV = DefaultFOV;
	}
	if (Character->HasAuthority())
	{
		InitializeCarriedAmmo();
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (Character == nullptr || !Character->IsLocallyControlled()) return;
	SetHUDCrosshairs(DeltaTime);
	
	FHitResult Hit;
	TraceUnderCrosshairs(Hit);
	HitTarget = Hit.ImpactPoint;

	InterpFOV(DeltaTime);
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if (!CanFire()) return;
	bCanFire = false;
	ServerFire(HitTarget);
	CrosshairShootingFactor = EquippedWeapon ? 0.75f : CrosshairShootingFactor;
	StartFireTimer();
}

bool UCombatComponent::CanFire() const
{
	if (EquippedWeapon == nullptr) return false;
	return bCanFire && !EquippedWeapon->IsEmpty() && CombatState == ECombatStates::ECS_Unoccupied;
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ThisClass::FireTimerFinished, EquippedWeapon->FireDelay);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character && EquippedWeapon && CombatState == ECombatStates::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	if (EquippedWeapon)
	{
		EquippedWeapon->Drop();
	}
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDWeaponAmmo();

	OnRep_EquippedWeapon();
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}
	
	EWeaponType EquippedWeaponType = EquippedWeapon->GetWeaponType();
	if (!CarriedAmmoMap.Contains(EquippedWeaponType)) return;
	CarriedAmmo = CarriedAmmoMap[EquippedWeaponType];
	UpdateCarriedAmmoHUD();
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	
	EquippedWeapon->SetWeaponState(EWeaponStates::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
	if (EquippedWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->EquipSound, Character->GetActorLocation());
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, InitialCarriedAmmo);
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	UpdateCarriedAmmoHUD();
}

void UCombatComponent::UpdateCarriedAmmoHUD()
{
	PlayerController = PlayerController == nullptr ? Cast<AMPPlayerController>(Character->GetController()) : PlayerController;
	if (PlayerController == nullptr) return;
	PlayerController->SetHUDCarriedAmmo(CarriedAmmo);
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo <= 0 || CombatState == ECombatStates::ECS_Reloading) return;
	ServerReload();
}

void UCombatComponent::ServerReload_Implementation()
{
	CombatState = ECombatStates::ECS_Reloading;
	HandleReload();
}
void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatStates::ECS_Reloading:
		HandleReload();
		break;
	case ECombatStates::ECS_Unoccupied:
		if (bFireButtonPressed) Fire();
		break;
	default:
		break;
	}
}

void UCombatComponent::HandleReload()
{
	if (Character == nullptr) return;
	Character->PlayReloadMontage();
}

void UCombatComponent::FinishReloading()
{
	if (Character == nullptr) return;
	if (Character->HasAuthority())
	{
		CombatState = ECombatStates::ECS_Unoccupied;
		UpdateAmmoAfterReload();
	}
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::UpdateAmmoAfterReload()
{
	if (EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	EquippedWeapon->AddAmmo(ReloadAmount);
	UpdateCarriedAmmoHUD();
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	EWeaponType EquippedWeaponType = EquippedWeapon->GetWeaponType();
	if (!CarriedAmmoMap.Contains(EquippedWeaponType)) return 0;

	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	int32 AmountCarried = CarriedAmmoMap[EquippedWeaponType];
	return FMath::Min(AmountCarried, RoomInMag);
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2d ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);
	
	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 80.f);
		}
		FVector End = CrosshairWorldPosition + CrosshairWorldDirection * TRACE_LENGTH;
		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECC_Visibility);
		HUDPackage.CrosshairColor = TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairInterface>()
										? FLinearColor::Red
										: FLinearColor::Green;
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	PlayerController = PlayerController == nullptr ? Cast<AMPPlayerController>(Character->Controller) : PlayerController;
	if (PlayerController == nullptr) return;
	
	HUD = HUD == nullptr ? Cast<AMPHUD>(PlayerController->GetHUD()) : HUD;
	if (HUD == nullptr) return;
	
	HUDPackage.CrosshairCenter = EquippedWeapon ? EquippedWeapon->CrosshairCenter : nullptr;
	HUDPackage.CrosshairLeft = EquippedWeapon ? EquippedWeapon->CrosshairLeft : nullptr;
	HUDPackage.CrosshairRight = EquippedWeapon ? EquippedWeapon->CrosshairRight : nullptr;
	HUDPackage.CrosshairTop = EquippedWeapon ? EquippedWeapon->CrosshairTop : nullptr;
	HUDPackage.CrosshairBottom = EquippedWeapon ? EquippedWeapon->CrosshairBottom : nullptr;

	FVector2d SpeedRange = FVector2d(0.f, Character->bIsCrouched ? Character->GetCharacterMovement()->MaxWalkSpeedCrouched : Character->GetCharacterMovement()->MaxWalkSpeed);
	FVector Velocity = Character->GetVelocity();
	Velocity.Z = 0.f;
	
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(SpeedRange, FVector2d(0.f, 1.f), Velocity.Size());

	CrosshairInAirFactor = Character->GetCharacterMovement()->IsFalling()
		                       ? CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f)
		                       : CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);

	CrosshairAimFactor = bAiming
		                     ? FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f)
		                     : FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);

	CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);
	
	HUDPackage.CrosshairSpread = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
	HUD->SetHUDPackage(HUDPackage);
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;
	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, UnZoomInterpSpeed);
	}
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}