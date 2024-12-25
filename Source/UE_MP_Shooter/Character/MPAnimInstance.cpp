// Fill out your copyright notice in the Description page of Project Settings.


#include "MPAnimInstance.h"

#include "MPCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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

	FRotator AimRotation = MPCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MPCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotator = FMath::RInterpTo(DeltaRotator, DeltaRot, DeltaSeconds, 6.f);
	YawOffset = DeltaRotator.Yaw;

	CharacterRotatorLastFrame = CharacterRotator;
	CharacterRotator = MPCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotator, CharacterRotatorLastFrame);
	const float Target = Delta.Yaw / DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = MPCharacter->GetAO_Yaw();
	AO_Pitch = MPCharacter->GetAO_Pitch();
}
