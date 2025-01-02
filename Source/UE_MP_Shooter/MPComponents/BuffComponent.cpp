#include "BuffComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "UE_MP_Shooter/Character/MPCharacter.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;


}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealthRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::HealthRampUp(float DeltaTime)
{
	if (!bHealing || Character == nullptr || Character->IsEliminated()) return;

	const float HealAmount = HealingRate * DeltaTime;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealAmount, 0, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealAmount;

	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
	bShieldReplenish = true;
	ShieldReplenishRate = ShieldAmount / ReplenishTime;
	ShieldReplenishAmount += ShieldAmount;
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (!bShieldReplenish || Character == nullptr || Character->IsEliminated()) return;

	const float ShieldAmount = ShieldReplenishRate * DeltaTime;
	Character->SetShield(FMath::Clamp(Character->GetShield() + ShieldAmount, 0, Character->GetMaxShield()));
	Character->UpdateHUDShield();
	ShieldReplenishAmount -= ShieldAmount;

	if (ShieldReplenishAmount <= 0.f || Character->GetShield() >= Character->GetMaxShield())
	{
		bShieldReplenish = false;
		ShieldReplenishAmount = 0.f;
	}
}

void UBuffComponent::SetInitialSpeed(float WalkSpeed, float CrouchSpeed, float JumpSpeed)
{
	InitialWalkSpeed = WalkSpeed;
	InitialCrouchSpeed = CrouchSpeed;
	InitialJumpSpeed = JumpSpeed;
}

void UBuffComponent::BuffSpeed(float WalkBuffSpeed, float CrouchBuffSpeed, float BuffTime)
{
	if (Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &ThisClass::ResetSpeed, BuffTime);
	MulticastSpeedBuff_Implementation(WalkBuffSpeed, CrouchBuffSpeed);
	MulticastSpeedBuff(WalkBuffSpeed, CrouchBuffSpeed);
}

void UBuffComponent::ResetSpeed()
{
	MulticastSpeedBuff_Implementation(InitialWalkSpeed, InitialCrouchSpeed);
	MulticastSpeedBuff(InitialWalkSpeed, InitialCrouchSpeed);
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float WalkSpeed, float CrouchSpeed)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	Character->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
}

void UBuffComponent::BuffJump(float JumpSpeed, float BuffTime)
{
	if (Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(JumpBuffTimer, this, &ThisClass::ResetJump, BuffTime);
	MulticastJumpBuff_Implementation(JumpSpeed);
	MulticastJumpBuff(JumpSpeed);
}

void UBuffComponent::ResetJump()
{
	MulticastJumpBuff_Implementation(InitialJumpSpeed);
	MulticastJumpBuff(InitialJumpSpeed);
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpSpeed)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	Character->GetCharacterMovement()->JumpZVelocity = JumpSpeed;
}