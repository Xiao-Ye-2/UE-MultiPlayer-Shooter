// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"

#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UE_MP_Shooter/Character/MPCharacter.h"
#include "UE_MP_Shooter/MPComponents/LagCompensationComponent.h"
#include "UE_MP_Shooter/PlayerController/MPPlayerController.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->SetIsReplicated(true);
	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed = InitialSpeed;
}

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed) && ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = InitialSpeed;
		ProjectileMovement->MaxSpeed = InitialSpeed;
	}
}
#endif

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();
	
	// FPredictProjectilePathParams PathParams;
	// PathParams.bTraceWithChannel = true;
	// PathParams.bTraceWithCollision = true;
	// PathParams.DrawDebugTime = 5.f;
	// PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	// PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	// PathParams.MaxSimTime = 4.f;
	// PathParams.ProjectileRadius = 5.f;
	// PathParams.SimFrequency = 30.f;
	// PathParams.StartLocation = GetActorLocation();
	// PathParams.TraceChannel = ECC_Visibility;
	// PathParams.ActorsToIgnore.Add(this);
	//
	// FPredictProjectilePathResult PathResult;
	//
	// UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpluse, const FHitResult& Hit)
{
	AMPCharacter* OwnerCharacter = Cast<AMPCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		AMPPlayerController* OwnerController = Cast<AMPPlayerController>(OwnerCharacter->GetController());
		if (OwnerController)
		{
			if (OwnerCharacter->HasAuthority() && !bUseServerSideRewind)
			{
				// const float DamageToCause = Hit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;
				const float DamageToCause = Damage;
				UGameplayStatics::ApplyDamage(OtherActor, DamageToCause, OwnerController, this, UDamageType::StaticClass());
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpluse, Hit);
				return;
			}
			AMPCharacter* HitCharacter = Cast<AMPCharacter>(OtherActor);
			if (bUseServerSideRewind && OwnerCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled() && HitCharacter)
			{
				OwnerCharacter->GetLagCompensationComponent()->ProjectileServerScoreRequest(HitCharacter, TraceStart,
					InitialVelocity, OwnerController->GetServerTime() - OwnerController->SingleTripTime);
			}
		}
	}
	
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpluse, Hit);
}
