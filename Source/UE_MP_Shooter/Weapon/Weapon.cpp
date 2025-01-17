// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Casing.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "UE_MP_Shooter/Character/MPCharacter.h"
#include "UE_MP_Shooter/MPComponents/CombatComponent.h"
#include "UE_MP_Shooter/PlayerController/MPPlayerController.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	AActor::SetReplicateMovement(true);
	
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	SetDefaultCustomDepthEnabled();

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
	
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);
}

void AWeapon::SetDefaultCustomDepthEnabled()
{
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnabledCustomDepth(true);
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		OwnerCharacter = nullptr;
		OwnerController = nullptr;
	} else
	{
		OwnerCharacter = OwnerCharacter == nullptr ? Cast<AMPCharacter>(Owner) : OwnerCharacter;
		if (OwnerCharacter && OwnerCharacter->GetEquippedWeapon() && OwnerCharacter->GetEquippedWeapon() == this) SetHUDWeaponAmmo();
	}
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMPCharacter* character = Cast<AMPCharacter>(OtherActor);
	if (character)
	{
		character->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMPCharacter* character = Cast<AMPCharacter>(OtherActor);
	if (character)
	{
		character->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::SetWeaponState(EWeaponStates State)
{
	WeaponState = State;
	OnRep_WeaponState();
}

void AWeapon::OnRep_WeaponState()
{
	if (WeaponState == EWeaponStates::EWS_Equipped)
		OnEquipped();
	else if (WeaponState == EWeaponStates::EWS_EquippedSecondary)
		OnEquippedSecondary();
	else if (WeaponState == EWeaponStates::EWS_Dropped)
		OnDropped();
}

void AWeapon::OnEquipped()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	EnabledCustomDepth(false);

	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AMPCharacter>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter == nullptr || !HasAuthority() || !bUseServerSideRewind) return;
	OwnerController = OwnerController == nullptr ? Cast<AMPPlayerController>(OwnerCharacter->GetController()) : OwnerController;
	if (OwnerController == nullptr || OwnerController->HighPingDelegate.IsBound()) return;
	OwnerController->HighPingDelegate.AddDynamic(this, &ThisClass::OnPingHigh);
}

void AWeapon::OnEquippedSecondary()
{
	OnEquipped();
	EnabledCustomDepth(true);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
	WeaponMesh->MarkRenderStateDirty();
}

void AWeapon::OnDropped()
{
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	SetDefaultCustomDepthEnabled();

	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AMPCharacter>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter == nullptr || !HasAuthority() || !bUseServerSideRewind) return;
	OwnerController = OwnerController == nullptr ? Cast<AMPPlayerController>(OwnerCharacter->GetController()) : OwnerController;
	if (OwnerController == nullptr || !OwnerController->HighPingDelegate.IsBound()) return;
	OwnerController->HighPingDelegate.RemoveDynamic(this, &ThisClass::OnPingHigh);
}

void AWeapon::Drop()
{
	SetWeaponState(EWeaponStates::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	OwnerCharacter = nullptr;
	OwnerController = nullptr;
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDWeaponAmmo();
	if (HasAuthority()) ClientUpdateAmmo(Ammo);
	else Sequence++;
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDWeaponAmmo();
}

void AWeapon::AddAmmo(int32 Amount)
{
	Ammo = FMath::Clamp(Ammo + Amount, 0, MagCapacity);
	SetHUDWeaponAmmo();
	ClientAddAmmo(Amount);
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AMPCharacter>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter && OwnerCharacter->GetCombatComponent() && IsFull())
	{
		OwnerCharacter->GetCombatComponent()->JumpToShotgunEnd();
	}
	SetHUDWeaponAmmo();
}

void AWeapon::SetHUDWeaponAmmo()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AMPCharacter>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter == nullptr) return;
	OwnerController = OwnerController == nullptr ? Cast<AMPPlayerController>(OwnerCharacter->GetController()) : OwnerController;
	if (OwnerController == nullptr) return;

	OwnerController->SetHUDWeaponAmmo(Ammo);
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		UWorld* World = GetWorld();
		if (AmmoEjectSocket && World)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(GetWeaponMesh());
			World->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
		}
	}
	SpendRound();
}

void AWeapon::EnabledCustomDepth(bool bEnabled) const
{
	if (WeaponMesh == nullptr) return;
	WeaponMesh->SetRenderCustomDepth(bEnabled);
}


FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget) const
{
	const USkeletalMeshSocket* MuzzleFlashSocket  = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return FVector();
	
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalize = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalize * DistanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector Dir = EndLoc - TraceStart;
	
	return TraceStart + Dir / Dir.Size() * TRACE_LENGTH;
}


void AWeapon::OnPingHigh(bool bPingHigh)
{
	bUseServerSideRewind = !bPingHigh;	
}