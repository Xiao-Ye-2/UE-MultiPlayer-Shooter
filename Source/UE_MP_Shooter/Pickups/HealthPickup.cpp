// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "UE_MP_Shooter/Character/MPCharacter.h"
#include "UE_MP_Shooter/MPComponents/BuffComponent.h"

AHealthPickup::AHealthPickup()
{
	bReplicates = true;
}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (AMPCharacter* MPCharacter = Cast<AMPCharacter>(OtherActor))
	{
		if (UBuffComponent* BuffComponent = MPCharacter->GetBuffComponent())
		{
			BuffComponent->Heal(HealAmount, HealingTime);
		}
	}
	Destroy();
}