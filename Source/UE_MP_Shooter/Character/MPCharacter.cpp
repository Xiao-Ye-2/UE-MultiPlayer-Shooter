// Fill out your copyright notice in the Description page of Project Settings.


#include "MPCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "UE_MP_Shooter/UE_MP_Shooter.h"
#include "UE_MP_Shooter/GameMode/BlasterGameMode.h"
#include "UE_MP_Shooter/MPComponents/CombatComponent.h"
#include "UE_MP_Shooter/MPComponents/BuffComponent.h"
#include "UE_MP_Shooter/MPComponents/LagCompensationComponent.h"
#include "UE_MP_Shooter/PlayerController/MPPlayerController.h"
#include "UE_MP_Shooter/PlayerState/MPPlayerState.h"
#include "UE_MP_Shooter/Weapon/Weapon.h"

AMPCharacter::AMPCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	BuffComponent->SetIsReplicated(true);

	LagCompensationComponent = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachedGrenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("RightHandSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 850.f, 0.f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	/**
	 * Hit boxes
	 */
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"), blanket);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), backpack);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AMPCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AMPCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AMPCharacter, Health);
	DOREPLIFETIME(AMPCharacter, Shield);
	DOREPLIFETIME(AMPCharacter, bDisableGameplay);
}

void AMPCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnDefaultWeapon();

	if (CombatComponent) CombatComponent->UpdateCarriedAmmoHUD();
	if (CombatComponent && CombatComponent->EquippedWeapon) CombatComponent->EquippedWeapon->SetHUDWeaponAmmo();
	UpdateHUDHealth();
	UpdateHUDShield();
	if (HasAuthority()) OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	if (AttachedGrenade) AttachedGrenade->SetVisibility(false);
}

void AMPCharacter::Destroyed()
{
	Super::Destroyed();

	if (EliminationEffectComponent)
	{
		EliminationEffectComponent->DestroyComponent();
	}

	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;
	if (CombatComponent && CombatComponent->EquippedWeapon && bMatchNotInProgress)
	{
		CombatComponent->EquippedWeapon->Destroy();
	}
}

void AMPCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();
	PollAndInitialize();
}

void AMPCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	if (GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f) OnRep_ReplicatedMovement();
		CalculateAO_Pitch();
	}
}

void AMPCharacter::HideCameraIfCharacterClose() const
{
	if (!IsLocallyControlled()) return;
	if (FollowCamera && (FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void AMPCharacter::PollAndInitialize()
{
	if (MPPlayerState == nullptr)
	{
		MPPlayerState = GetPlayerState<AMPPlayerState>();
		if (MPPlayerState)
		{
			MPPlayerState->AddToScore(0.f);
			MPPlayerState->AddToDefeats(0);
		}
	}
}

void AMPCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMPCharacter::Jump);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMPCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMPCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AMPCharacter::LookUp);
	PlayerInputComponent->BindAxis("Turn", this, &AMPCharacter::Turn);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AMPCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMPCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AMPCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AMPCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMPCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AMPCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AMPCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &AMPCharacter::GrenadeButtonPressed);
}

void AMPCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent) CombatComponent->Character = this;
	if (BuffComponent)
	{
		BuffComponent->Character = this;
		BuffComponent->SetInitialSpeed(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched, GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensationComponent)
	{
		LagCompensationComponent->Character = this;
		if (Controller) LagCompensationComponent->Controller = Cast<AMPPlayerController>(Controller);
	}
}

void AMPCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void AMPCharacter::Eliminate(bool bPlayerLeftGame)
{
	if (CombatComponent)
	{
		WeaponHandleWhenEliminated(CombatComponent->EquippedWeapon);
		WeaponHandleWhenEliminated(CombatComponent->SecondaryWeapon);
	}
	MulticastEliminate(bPlayerLeftGame);
}

void AMPCharacter::WeaponHandleWhenEliminated(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;
	if (Weapon->bDestroyWeaponWhenDrop) Weapon->Destroy();
	else Weapon->Drop();
}

void AMPCharacter::MulticastEliminate_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	if (MPPlayerController) MPPlayerController->SetHUDWeaponAmmo(0);
	bEliminated = true;
	PlayEliminateMontage();

	// Dissolve Effects
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		// DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	// Disable Movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	bDisableGameplay = true;
	if (CombatComponent) CombatComponent->FireButtonPressed(false);
	
	// Disable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Spawn Elimination Effect
	if (EliminationEffect)
	{
		FVector EliminationEffectLocation(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		EliminationEffectComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EliminationEffect, EliminationEffectLocation, GetActorRotation());
	}
	if (EliminationEffectSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, EliminationEffectSound, GetActorLocation());
	}

	if (IsLocallyControlled() && CombatComponent && CombatComponent->bAiming && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		ShowSniperScopeWidget(false);
	}

	GetWorldTimerManager().SetTimer(EliminatedTimer, this, &ThisClass::EliminateTimerFinished, EliminateDelay);
}

void AMPCharacter::EliminateTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterGameMode && !bLeftGame)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
	else if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

void AMPCharacter::PlayFireMontage(bool bAiming)
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMPCharacter::PlayReloadMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (CombatComponent->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		default:
			SectionName = FName("Rifle");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMPCharacter::PlayEliminateMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance == nullptr || ElimMontage == nullptr) return;
	AnimInstance->Montage_Play(ElimMontage);
}

void AMPCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance == nullptr || ThrowGrenadeMontage == nullptr) return;
	AnimInstance->Montage_Play(ThrowGrenadeMontage);
}

void AMPCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance == nullptr || SwapMontage == nullptr) return;
	AnimInstance->Montage_Play(SwapMontage);
}

void AMPCharacter::PlayHitReactMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance == nullptr || HitReactMontage == nullptr) return;
	AnimInstance->Montage_Play(HitReactMontage);
	FName SectionName = FName("FromFront");
	AnimInstance->Montage_JumpToSection(SectionName);
}

void AMPCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0.0f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction( FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) );
		AddMovementInput(Direction, Value);
	}
}

void AMPCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0.0f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction( FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y) );
		AddMovementInput(Direction, Value);
	}
}

void AMPCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AMPCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AMPCharacter::Jump()
{
	if (bDisableGameplay) return;
	if (bIsCrouched)
	{
		UnCrouch();
	} else
	{
		Super::Jump();
	}
}

void AMPCharacter::EquipButtonPressed()
{
	if (bDisableGameplay || CombatComponent == nullptr) return;
	if (CombatComponent->CombatState == ECombatStates::ECS_Unoccupied) ServerEquipButtonPressed();
	if (CombatComponent->ShouldSwapWeapons() && !HasAuthority()
		&& CombatComponent->CombatState == ECombatStates::ECS_Unoccupied && OverlappingWeapon == nullptr)
	{
		PlaySwapMontage();
		bFinishedSwapping = false;
		CombatComponent->CombatState = ECombatStates::ECS_SwappingWeapons;
	}
}

void AMPCharacter::ServerEquipButtonPressed_Implementation()
{
	if (CombatComponent == nullptr) return;
	if (OverlappingWeapon)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	} else if (CombatComponent->ShouldSwapWeapons())
	{
		CombatComponent->SwapWeapons();
	}
}

void AMPCharacter::CrouchButtonPressed()
{
	if (bDisableGameplay) return;
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AMPCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return;
	if (CombatComponent)
	{
		CombatComponent->Reload();
	}
}

void AMPCharacter::AimButtonPressed()
{
	if (bDisableGameplay) return;
	if (CombatComponent)
	{
		CombatComponent->SetAiming(true);
	}
}

void AMPCharacter::AimButtonReleased()
{
	if (bDisableGameplay) return;
	if (CombatComponent)
	{
		CombatComponent->SetAiming(false);
	}
}

float AMPCharacter::CalculateSpeed() const
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	return Velocity.Size();
}

void AMPCharacter::AimOffset(float DeltaTime)
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	// bUseControllerRotationYaw = true;

	if (Speed == 0.f && !bIsInAir)
	{
		bRotateRootBone = true;
		FRotator CurrentRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentRotation, StartingAimRotation);
		AO_Yaw = DeltaRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			Interp_AO_Yaw = AO_Yaw;
		}
		TurnInPlace(DeltaTime);
	} else
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	
	CalculateAO_Pitch();
}

void AMPCharacter::SimProxiesTurn()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	
	if (CalculateSpeed() > 0.f) return;

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		TurningInPlace = ProxyYaw > 0
			                 ? ETurningInPlace::ETIP_Right
			                 : ETurningInPlace::ETIP_Left;
	}
}

void AMPCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled()) // Data compression when passing around by client and server
	{
		FVector2d InRange(270.f, 360.f);
		FVector2d OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AMPCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		Interp_AO_Yaw = FMath::FInterpTo(Interp_AO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = Interp_AO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void AMPCharacter::FireButtonPressed()
{
	if (bDisableGameplay) return;
	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(true);
	}
}

void AMPCharacter::FireButtonReleased()
{
	if (bDisableGameplay) return;
	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(false);
	}
}

void AMPCharacter::GrenadeButtonPressed()
{
	if (CombatComponent == nullptr) return;
	CombatComponent->ThrowGrenade();
}

void AMPCharacter::ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType,
                                 AController* InstigatorController, AActor* DamageCauser)
{
	if (bEliminated) return;
	if (Shield > 0.f && Shield >= Damage)
	{
		Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
	} else 
	{
		Health = FMath::Clamp(Health + Shield - Damage, 0.f, MaxHealth);
		Shield = 0.f;
	}
	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();

	if (Health <= 0.f)
	{
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if (BlasterGameMode)
		{
			MPPlayerController = MPPlayerController == nullptr ? Cast<AMPPlayerController>(Controller) : MPPlayerController;
			AMPPlayerController* AttackerController = Cast<AMPPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, MPPlayerController, AttackerController);
		}
	}
}

void AMPCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue("Dissolve", DissolveValue);
	}
}

void AMPCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ThisClass::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void AMPCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled() && OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
}

void AMPCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon) const
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void AMPCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth) PlayHitReactMontage();
}

void AMPCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield) PlayHitReactMontage();
}

void AMPCharacter::UpdateHUDHealth()
{
	MPPlayerController = MPPlayerController == nullptr ? Cast<AMPPlayerController>(GetController()) : MPPlayerController;
	if (MPPlayerController) MPPlayerController->SetHUDHealth(Health, MaxHealth);
}

void AMPCharacter::UpdateHUDShield()
{
	MPPlayerController = MPPlayerController == nullptr ? Cast<AMPPlayerController>(GetController()) : MPPlayerController;
	if (MPPlayerController) MPPlayerController->SetHUDShield(Shield, MaxShield);
}

void AMPCharacter::SpawnDefaultWeapon()
{
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if (BlasterGameMode && World && !bEliminated && DefaultWeaponClass && CombatComponent)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeaponWhenDrop = true;
		CombatComponent->EquipWeapon(StartingWeapon);
	}
}

void AMPCharacter::ServerLeaveGame_Implementation()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	MPPlayerState = MPPlayerState == nullptr ? GetPlayerState<AMPPlayerState>() : MPPlayerState;
	if (BlasterGameMode && MPPlayerState)
	{
		BlasterGameMode->PlayerLeftGame(MPPlayerState);
	}
}

bool AMPCharacter::IsWeaponEquipped() const
{
	return CombatComponent && CombatComponent->EquippedWeapon;	
}

bool AMPCharacter::IsAiming() const
{
	return CombatComponent && CombatComponent->bAiming;
}

AWeapon* AMPCharacter::GetEquippedWeapon() const
{
	return CombatComponent == nullptr ? nullptr : CombatComponent->EquippedWeapon;
}

FVector AMPCharacter::GetHitTarget() const
{
	return CombatComponent == nullptr ? FVector() : CombatComponent->HitTarget;
}

ECombatStates AMPCharacter::GetCombatState() const
{
	return CombatComponent == nullptr ? ECombatStates::ECS_MAX : CombatComponent->CombatState;
}
