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
	FVector End = Start + (HitTarget - Start) * 1.25f;
	FHitResult Hit;
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	if (MuzzleFlashParticleSystem)
	{
		UGameplayStatics::SpawnEmitterAtLocation(World, MuzzleFlashParticleSystem, SocketTransform);
	}
	if (FireSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, FireSound, GetActorLocation());
	}
	World->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility);
	FVector BeamEnd = Hit.bBlockingHit ? Hit.ImpactPoint : End;
	if (BeamParticleSystem)
	{
		UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticleSystem, SocketTransform);
		if (Beam) Beam->SetVectorParameter(FName("Target"), BeamEnd);
	}
	if (!Hit.bBlockingHit) return;
	
	AMPCharacter* MPCharacter = Cast<AMPCharacter>(Hit.GetActor());
	if (MPCharacter && HasAuthority() && OwnerPawnController)
	{
		UGameplayStatics::ApplyDamage(MPCharacter, Damage, OwnerPawnController, this, UDamageType::StaticClass());
	}
	if (ImpactParticleSystem)
	{
		UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticleSystem, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, Hit.ImpactPoint);
	}
	
}
