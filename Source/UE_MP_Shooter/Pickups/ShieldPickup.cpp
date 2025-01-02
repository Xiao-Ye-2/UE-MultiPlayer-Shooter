// Fill out your copyright notice in the Description page of Project Settings.


#include "ShieldPickup.h"

#include "UE_MP_Shooter/Character/MPCharacter.h"
#include "UE_MP_Shooter/MPComponents/BuffComponent.h"

void AShieldPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (AMPCharacter* MPCharacter = Cast<AMPCharacter>(OtherActor))
	{
		if (UBuffComponent* BuffComponent = MPCharacter->GetBuffComponent())
		{
			BuffComponent->ReplenishShield(ShieldReplenishAmount, ShieldReplenishTime);
		}
	}
	Destroy();
}
