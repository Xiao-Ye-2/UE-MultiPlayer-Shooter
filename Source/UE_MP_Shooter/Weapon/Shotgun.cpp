// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "UE_MP_Shooter/Character/MPCharacter.h"

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* OwnerPawnController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket  = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;
	
	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	FVector Start = SocketTransform.GetLocation();
	TMap<AMPCharacter*, uint32> BulletsHitMap;

	for (auto HitTarget : HitTargets)
	{
		FHitResult Hit;
		WeaponTraceHit(Hit, Start, HitTarget, 0.5f, FMath::FRandRange(-.5f, .5f));
		if (AMPCharacter* MPCharacter = Cast<AMPCharacter>(Hit.GetActor()))
		{
			if (BulletsHitMap.Contains(MPCharacter)) BulletsHitMap[MPCharacter] += 1;
			else BulletsHitMap.Emplace(MPCharacter, 1);
		}
	}
	for (auto HitPair : BulletsHitMap)
	{
		if (HitPair.Key && HasAuthority() && OwnerPawnController)
		{
			UGameplayStatics::ApplyDamage(HitPair.Key, Damage * HitPair.Value, OwnerPawnController, this, UDamageType::StaticClass());
		}
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket  = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;
	
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	const FVector ToTargetNormalize = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalize * DistanceToSphere;

	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		const FVector Dir = EndLoc - TraceStart;
		
		HitTargets.Add(TraceStart + Dir / Dir.Size() * TRACE_LENGTH);
	}
}
