// Fill out your copyright notice in the Description page of Project Settings.


#include "MPAnimInstance.h"
#include "MPCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMPAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	MPCharacter = Cast<AMPCharacter>(TryGetPawnOwner());
}

void UMPAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (MPCharacter == nullptr)
	{
		MPCharacter = Cast<AMPCharacter>(TryGetPawnOwner());
	}
	if (MPCharacter == nullptr) return;

	FVector Velocity = MPCharacter->GetVelocity();
	Velocity.Z = 0.0f;
	Speed = Velocity.Size();

	bIsInAir = MPCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = MPCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.0f;
	bWeaponEquipped = MPCharacter->IsWeaponEquipped();
	bIsCrouched = MPCharacter->bIsCrouched;
	bAiming = MPCharacter->IsAiming();
}
