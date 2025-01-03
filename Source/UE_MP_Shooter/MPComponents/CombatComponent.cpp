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
#include "UE_MP_Shooter/Weapon/Projectile.h"
#include "UE_MP_Shooter/Weapon/Shotgun.h"

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
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
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

bool UCombatComponent::CanFire() const
{
	if (EquippedWeapon == nullptr || bLocallyReloading) return false;
	if (!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatStates::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun) return true;
	return bCanFire && !EquippedWeapon->IsEmpty() && CombatState == ECombatStates::ECS_Unoccupied;
}

void UCombatComponent::Fire()
{
	if (!CanFire()) return;
	bCanFire = false;
	StartFireTimer();
	if (EquippedWeapon == nullptr) return;
	
	CrosshairShootingFactor = 0.75f;
	if (EquippedWeapon->FireType == EFireType::EFT_Projectile || EquippedWeapon->FireType == EFireType::EFT_HitScan)
		FireSingleBulletWeapon();
	else if (EquippedWeapon->FireType == EFireType::EFT_Shotgun) 
		FireShotgunWeapon();
}

void UCombatComponent::FireSingleBulletWeapon()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	HitTarget = EquippedWeapon->IsUsingScatter() ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
	if (!Character->HasAuthority()) LocalFire(HitTarget);
	ServerFire(HitTarget);
}

void UCombatComponent::FireShotgunWeapon()
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun == nullptr || Character == nullptr) return;
	TArray<FVector_NetQuantize> HitTargets;
	Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
	if (!Character->HasAuthority()) LocalShotgunFire(HitTargets);
	ServerShotgunFire(HitTargets);
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
	ReloadIfWeaponEmpty();
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalFire(TraceHitTarget);
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character == nullptr || EquippedWeapon == nullptr || CombatState != ECombatStates::ECS_Unoccupied) return;
	Character->PlayFireMontage(bAiming);
	EquippedWeapon->Fire(TraceHitTarget);
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	MulticastShotgunFire(TraceHitTargets);
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalShotgunFire(TraceHitTargets);
}

void UCombatComponent::LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Character == nullptr || Shotgun == nullptr) return;
	if (CombatState == ECombatStates::ECS_Unoccupied || CombatState == ECombatStates::ECS_Reloading)
	{
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState = ECombatStates::ECS_Unoccupied;
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr || CombatState != ECombatStates::ECS_Unoccupied) return;

	if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
	{
		EquipSecondaryWeapon(WeaponToEquip);
	} else
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	DropEquippedWeapon();
	
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetOwner(Character);

	OnRep_PrimaryWeapon();
	ReloadIfWeaponEmpty();
	UpdateCarriedAmmo();
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	SecondaryWeapon = WeaponToEquip;
	OnRep_SecondaryWeapon();
	SecondaryWeapon->SetOwner(Character);
}

void UCombatComponent::OnRep_PrimaryWeapon()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	
	EquippedWeapon->SetWeaponState(EWeaponStates::EWS_Equipped);
	EquippedWeapon->SetHUDWeaponAmmo();
	AttachActorToRightHand(EquippedWeapon);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
	PlayWeaponEquipSound(EquippedWeapon);
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon == nullptr || Character == nullptr) return;
	SecondaryWeapon->SetWeaponState(EWeaponStates::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryWeapon);
	PlayWeaponEquipSound(SecondaryWeapon);
}

void UCombatComponent::DropEquippedWeapon() const
{
	if (EquippedWeapon == nullptr) return;
	EquippedWeapon->Drop();
}

void UCombatComponent::SwapWeapons()
{
	if (EquippedWeapon == nullptr || SecondaryWeapon == nullptr || CombatState != ECombatStates::ECS_Unoccupied) return;
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;

	EquippedWeapon->SetWeaponState(EWeaponStates::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetHUDWeaponAmmo();
	UpdateCarriedAmmo();
	PlayWeaponEquipSound(EquippedWeapon);

	SecondaryWeapon->SetWeaponState(EWeaponStates::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryWeapon);
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach) const
{
	AttachActorToSocket(ActorToAttach, FName("RightHandSocket"));
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach) const
{
	bool bUsePistolSocket = EquippedWeapon && (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol || EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun);
	FName LeftSocketName = bUsePistolSocket ? FName("PistolSocket") : FName("LeftHandSocket");
	AttachActorToSocket(ActorToAttach, LeftSocketName);
}

void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach) const
{
	AttachActorToSocket(ActorToAttach, FName("BackPackSocket"));
}

void UCombatComponent::AttachActorToSocket(AActor* ActorToAttach, FName SocketName) const
{
	if (Character == nullptr || Character->GetMesh() == nullptr  || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket == nullptr) return;
	HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
}

void UCombatComponent::PlayWeaponEquipSound(AWeapon* WeaponToEquip) const
{
	if (Character == nullptr || WeaponToEquip == nullptr || WeaponToEquip->EquipSound == nullptr) return;
	UGameplayStatics::PlaySoundAtLocation(this, WeaponToEquip->EquipSound, Character->GetActorLocation());
}

void UCombatComponent::ReloadIfWeaponEmpty()
{
	if (EquippedWeapon == nullptr || !EquippedWeapon->IsEmpty()) return;
	Reload();
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;
	EWeaponType EquippedWeaponType = EquippedWeapon->GetWeaponType();
	if (!CarriedAmmoMap.Contains(EquippedWeaponType)) return;
	CarriedAmmo = CarriedAmmoMap[EquippedWeaponType];
	UpdateCarriedAmmoHUD();
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, InitialRifleCarriedAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, InitialRocketCarriedAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, InitialPistolCarriedAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, InitialSMGCarriedAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, InitialShotgunCarriedAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, InitialSniperCarriedAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, InitialGrenadeLauncherCarriedAmmo);
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	UpdateCarriedAmmoHUD();
	if (CombatState == ECombatStates::ECS_Reloading && EquippedWeapon != nullptr && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun && CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::UpdateCarriedAmmoHUD()
{
	PlayerController = PlayerController == nullptr ? Cast<AMPPlayerController>(Character->GetController()) : PlayerController;
	if (PlayerController == nullptr) return;
	PlayerController->SetHUDCarriedAmmo(CarriedAmmo);
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo <= 0 || CombatState != ECombatStates::ECS_Unoccupied || (EquippedWeapon && EquippedWeapon->GetAmmo() == EquippedWeapon->GetMagCapacity()) || bLocallyReloading) return;
	ServerReload();
	HandleReload();
	bLocallyReloading = true;
}

void UCombatComponent::ServerReload_Implementation()
{
	CombatState = ECombatStates::ECS_Reloading;
	if (!Character->IsLocallyControlled()) HandleReload();
}
void UCombatComponent::OnRep_CombatState()
{
	if (CombatState == ECombatStates::ECS_Reloading)
	{
		if (Character && !Character->IsLocallyControlled()) HandleReload();
	}
	else if (CombatState == ECombatStates::ECS_Unoccupied)
	{
		if (bFireButtonPressed) Fire();
	}
	else if (CombatState == ECombatStates::ECS_ThrowingGrenade)
	{
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
			ShowAttachedGrenade(true);
		}
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
	bLocallyReloading = false;
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

void UCombatComponent::ShotgunShellReload()
{
	if (Character && Character->HasAuthority()) UpdateShotgunAmmoAfterReload();
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


void UCombatComponent::UpdateShotgunAmmoAfterReload()
{
	if (EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	EquippedWeapon->AddAmmo(1);
	UpdateCarriedAmmoHUD();
	bCanFire = true;
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::JumpToShotgunEnd() const
{
	if (Character == nullptr || Character->GetReloadMontage() == nullptr) return;
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance == nullptr) return;
	AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
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

void UCombatComponent::ThrowGrenade()
{
	if (Grenades == 0 || Character == nullptr) return;
	ServerThrowGrenade_Implementation();
	if (!Character->HasAuthority())
	{
		ServerThrowGrenade();
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades == 0) return;
	if (CombatState != ECombatStates::ECS_Unoccupied || EquippedWeapon == nullptr) return;
	CombatState = ECombatStates::ECS_ThrowingGrenade;
	if (Character == nullptr) return;
	Character->PlayThrowGrenadeMontage();
	AttachActorToLeftHand(EquippedWeapon);
	ShowAttachedGrenade(true);
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	OnRep_Grenades();
}

void UCombatComponent::OnRep_Grenades()
{
	PlayerController = PlayerController == nullptr ? Cast<AMPPlayerController>(Character->Controller) : PlayerController;
	if (PlayerController == nullptr) return;
	PlayerController->SetHUDGrenades(Grenades);
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade) const
{
	if (Character == nullptr || Character->GetAttachedGrenade() == nullptr) return;
	Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatStates::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	if (Character && Character->IsLocallyControlled()) ServerLaunchGrenade(HitTarget);
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (GrenadeProjectileClass == nullptr || Character == nullptr || Character->GetAttachedGrenade() == nullptr) return;
	UWorld* World = Character->GetWorld();
	if (World == nullptr) return;
	
	const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
	FVector ToTarget = Target - StartingLocation;
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.Instigator = Character;
	World->SpawnActor<AProjectile>(GrenadeProjectileClass, StartingLocation, ToTarget.Rotation(), SpawnParams);
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 Amount)
{
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + Amount, 0, MaxCarriedAmmo);
	} else
	{
		CarriedAmmoMap.Emplace(WeaponType, Amount);
	}
	UpdateCarriedAmmo();
	
	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}
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

void UCombatComponent::OnRep_Aiming()
{
	if (Character == nullptr || !Character->IsLocallyControlled()) return;
	bAiming = bAimButtonPressed;
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return; 
	ServerSetAiming_Implementation(bIsAiming);
	ServerSetAiming(bIsAiming);
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
	if (Character->IsLocallyControlled()) bAimButtonPressed = bIsAiming;
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}