// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"

#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	if (MuzzleFlashSocket == nullptr || ProjectileClass == nullptr || InstigatorPawn == nullptr || World == nullptr) return;
	
	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	FVector ToTarget = HitTarget - SocketTransform.GetLocation();
	FRotator ToTargetRotation = ToTarget.Rotation();
	
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = InstigatorPawn;

	AProjectile* SpawnedProjectile = nullptr;
	if (bUseServerSideRewind)
	{
		if (InstigatorPawn->HasAuthority()) // Server, SSR
		{
			if (InstigatorPawn->IsLocallyControlled()) // Server, host - use Replicated projectile
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), ToTargetRotation, SpawnParameters);
				SpawnedProjectile->bUseServerSideRewind = false;
				SpawnedProjectile->Damage = Damage;
			}
			else // Server, not locally controlled - spawn non-replicated projectile, SSR
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(NoReplicationProjectileClass, SocketTransform.GetLocation(), ToTargetRotation, SpawnParameters);
				SpawnedProjectile->bUseServerSideRewind = true; // Server side rewind bullet doesn't cause damage on Server
			}
		}
		else // Client, SSR
		{
			if (InstigatorPawn->IsLocallyControlled()) // Client, locally controlled - spawn non-replicated projectile, use SSR
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(NoReplicationProjectileClass, SocketTransform.GetLocation(), ToTargetRotation, SpawnParameters);
				SpawnedProjectile->bUseServerSideRewind = true;
				SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
				SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
				SpawnedProjectile->Damage = Damage;
			} else // Client, not locally controlled - spawn non-replicated projectile, no SSR
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(NoReplicationProjectileClass, SocketTransform.GetLocation(), ToTargetRotation, SpawnParameters);
				SpawnedProjectile->bUseServerSideRewind = false;
			}
		}
	}
	else
	{
		if (InstigatorPawn->HasAuthority())
		{
			SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), ToTargetRotation, SpawnParameters);
			SpawnedProjectile->bUseServerSideRewind = false;
			SpawnedProjectile->Damage = Damage;
		}
	}
	
}
