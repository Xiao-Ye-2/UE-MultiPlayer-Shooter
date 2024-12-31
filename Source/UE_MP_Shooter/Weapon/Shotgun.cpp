// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "UE_MP_Shooter/Character/MPCharacter.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* OwnerPawnController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket  = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;
	
	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	FVector Start = SocketTransform.GetLocation();
	TMap<AMPCharacter*, uint32> BulletsHitMap;
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		FHitResult Hit;
		WeaponTraceHit(Hit, Start, HitTarget, 0.5f, FMath::FRandRange(-.5f, .5f));

		AMPCharacter* MPCharacter = Cast<AMPCharacter>(Hit.GetActor());
		if (MPCharacter && HasAuthority() && OwnerPawnController)
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
