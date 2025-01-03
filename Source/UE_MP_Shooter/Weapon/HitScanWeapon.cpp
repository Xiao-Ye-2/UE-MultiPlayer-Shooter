// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "UE_MP_Shooter/Character/MPCharacter.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* OwnerPawnController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket  = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;
	
	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	FVector Start = SocketTransform.GetLocation();
	FHitResult Hit;
	WeaponTraceHit(Hit, Start, HitTarget);

	if (MuzzleFlashParticleSystem)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlashParticleSystem, SocketTransform);
	}
	if (FireSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, FireSound, GetActorLocation());
	}
	
	AMPCharacter* MPCharacter = Cast<AMPCharacter>(Hit.GetActor());
	if (MPCharacter && HasAuthority() && OwnerPawnController)
	{
		UGameplayStatics::ApplyDamage(MPCharacter, Damage, OwnerPawnController, this, UDamageType::StaticClass());
	}
}

void AHitScanWeapon::WeaponTraceHit(FHitResult& Hit, const FVector& TraceStart, const FVector& HitTarget, float HitSoundVolumeMultiplier, float HitSoundPitchMultiplier)
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;
	
	FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;
	World->LineTraceSingleByChannel(Hit, TraceStart, End, ECC_Visibility);
	FVector BeamEnd = Hit.bBlockingHit ? Hit.ImpactPoint : End;
	if (BeamParticleSystem)
	{
		UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticleSystem, TraceStart, FRotator::ZeroRotator);
		if (Beam) Beam->SetVectorParameter(FName("Target"), BeamEnd);
	}
	if (!Hit.bBlockingHit) return;

	if (ImpactParticleSystem)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticleSystem, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, Hit.ImpactPoint, HitSoundVolumeMultiplier, HitSoundPitchMultiplier);
	}
}