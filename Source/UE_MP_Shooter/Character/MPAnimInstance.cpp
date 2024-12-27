// Fill out your copyright notice in the Description page of Project Settings.


#include "MPAnimInstance.h"

#include "MPCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "UE_MP_Shooter/Weapon/Weapon.h"

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
	EquippedWeapon = MPCharacter->GetEquippedWeapon();
	bIsCrouched = MPCharacter->bIsCrouched;
	bAiming = MPCharacter->IsAiming();
	TurningInPlace = MPCharacter->GetTurningInPlace();
	bLocallyControlled = MPCharacter->IsLocallyControlled();
	bRotateRootBone = MPCharacter->ShouldRotateRootBone();
	bEliminated = MPCharacter->IsEliminated();

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

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && MPCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		MPCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (MPCharacter->IsLocallyControlled())
		{
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_r"), RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), 2 * RightHandTransform.GetLocation() - MPCharacter->GetHitTarget());
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaSeconds, 20.f);
		}
	}
}
