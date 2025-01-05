
#include "LagCompensationComponent.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/MappedName.h"
#include "UE_MP_Shooter/UE_MP_Shooter.h"
#include "UE_MP_Shooter/Character/MPCharacter.h"
#include "UE_MP_Shooter/Weapon/Weapon.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}


void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character == nullptr || !Character->HasAuthority()) return;
	UpdateFramePackageHistory();
}

void ULagCompensationComponent::UpdateFramePackageHistory()
{
	FFramePackage Package;
	SaveFramePackage(Package);
	FrameHistory.AddHead(Package);

	float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
	while (HistoryLength > MaxRecordTime)
	{
		FrameHistory.RemoveNode(FrameHistory.GetTail());
		HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<AMPCharacter>(GetOwner()) : Character;
	if (Character == nullptr) return;
	Package.Time = GetWorld()->GetTimeSeconds();
	Package.Character = Character;
	for (auto& BoxPair : Character->HitCollisionBoxes)
	{
		FBoxInformation BoxInformation;
		BoxInformation.Location = BoxPair.Value->GetComponentLocation();
		BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
		BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
		Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color) const
{
	for (auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(GetWorld(), BoxInfo.Value.Location, BoxInfo.Value.BoxExtent, FQuat(BoxInfo.Value.Rotation), Color, false, 4.f);
	}
}

void ULagCompensationComponent::CacheBoxPositions(AMPCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;
	OutFramePackage.Character = HitCharacter;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value == nullptr) continue;
		FBoxInformation BoxInformation;
		BoxInformation.Location = HitBoxPair.Value->GetComponentLocation();
		BoxInformation.Rotation = HitBoxPair.Value->GetComponentRotation();
		BoxInformation.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
		OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInformation);
	}
}

void ULagCompensationComponent::MoveBoxes(AMPCharacter* HitCharacter, const FFramePackage& Package, bool DisableCollision)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value == nullptr || !Package.HitBoxInfo.Contains(HitBoxPair.Key)) continue;
		HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
		HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
		HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
		if (DisableCollision) HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(const AMPCharacter* HitCharacter,
	ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter == nullptr || HitCharacter->GetMesh() == nullptr) return;
	HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame,
                                                             const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);
	if (InterpFraction <= 0.f) return OlderFrame;
	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;
	InterpFramePackage.Character = YoungerFrame.Character;

	for (auto& YoungerBoxPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerBoxPair.Key;
		const FBoxInformation& OlderBoxInformation = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBoxInformation = YoungerBoxPair.Value;

		FBoxInformation InterpBoxInformation;
		InterpBoxInformation.Location = FMath::VInterpTo(OlderBoxInformation.Location, YoungerBoxInformation.Location, 1.f, InterpFraction);
		InterpBoxInformation.Rotation = FMath::RInterpTo(OlderBoxInformation.Rotation, YoungerBoxInformation.Rotation, 1.f, InterpFraction);
		InterpBoxInformation.BoxExtent = YoungerBoxInformation.BoxExtent;
		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInformation);
	}
	return InterpFramePackage;
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(AMPCharacter* HitCharacter, float HitTime)
{
	if (HitCharacter == nullptr || HitCharacter->GetLagCompensationComponent() == nullptr || HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr) return FFramePackage();
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory;
	const float OldTime = History.GetTail()->GetValue().Time;
	if (OldTime > HitTime) return FFramePackage(); // Too far back in time
	
	auto NewerPointer = History.GetHead();
	auto OlderPointer = NewerPointer;
	while (OlderPointer->GetValue().Time > HitTime)
	{
		if (OlderPointer->GetNextNode() == nullptr) break;
		OlderPointer = OlderPointer->GetNextNode();
		if (OlderPointer->GetValue().Time > HitTime) NewerPointer = OlderPointer;
	}
	
	FFramePackage FrameToCheck = HitTime >= NewerPointer->GetValue().Time
		? History.GetHead()->GetValue()
		: InterpBetweenFrames(OlderPointer->GetValue(), NewerPointer->GetValue(), HitTime);
	FrameToCheck.Character = HitCharacter; // Optional but safety guard.
	return FrameToCheck;
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, AMPCharacter* HitCharacter,
                                                              const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	UWorld* World = GetWorld();
	if (World == nullptr || HitCharacter == nullptr) return FServerSideRewindResult();
	FFramePackage CurrentFrame;
	FServerSideRewindResult Result = FServerSideRewindResult();
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	// Enable Collision for head
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Visibility);
	if (ConfirmHitResult.bBlockingHit && ConfirmHitResult.GetActor() == HitCharacter)
	{
		Result = FServerSideRewindResult{true, true};
	} else // Check the body
	{
		for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
		{
			if (HitBoxPair.Value == nullptr) continue;;
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			HitBoxPair.Value->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		}
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Visibility);
		if (ConfirmHitResult.bBlockingHit && ConfirmHitResult.GetActor() == HitCharacter) Result = FServerSideRewindResult{true, false};
	}
	MoveBoxes(HitCharacter, CurrentFrame, true);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return Result;
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages,
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	for (auto& Frame : FramePackages) if (Frame.Character == nullptr) {
		if (GEngine)
		{
			FString DebugMessage = TEXT("Character is nullptr for Frame");
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, DebugMessage);
		}
		return FShotgunServerSideRewindResult();
	};
	UWorld* World = GetWorld();
	if (World == nullptr) return FShotgunServerSideRewindResult();
	
	FShotgunServerSideRewindResult ShotgunResult;
	TArray<FFramePackage> CurrentFrames;
	TArray<AMPCharacter*> HitCharacters;
	for (auto& Frame : FramePackages)
	{
		FFramePackage CurrentFrame;
		CacheBoxPositions(Frame.Character, CurrentFrame);
		MoveBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::NoCollision);
		CurrentFrames.Add(CurrentFrame);
		HitCharacters.Emplace(Frame.Character);

		// Enable Collision for head
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}

	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Visibility);
		if (ConfirmHitResult.bBlockingHit)
		{
			AMPCharacter* HitCharacter = Cast<AMPCharacter>(ConfirmHitResult.GetActor());
			if (HitCharacter == nullptr || !HitCharacters.Contains(HitCharacter)) continue;
			
			if (ShotgunResult.HeadShots.Contains(HitCharacter)) ShotgunResult.HeadShots[HitCharacter]++;
			else ShotgunResult.HeadShots.Emplace(HitCharacter, 1);
		}
	}

	// Enable collision for all boxes, then disable for head box
	for (auto& Frame : FramePackages)
	{
		for (auto& HitBoxPair : Frame.Character->HitCollisionBoxes)
		{
			if (HitBoxPair.Value == nullptr) continue;
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
		}
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	// check for body shots
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Visibility);
		if (ConfirmHitResult.bBlockingHit)
		{
			AMPCharacter* HitCharacter = Cast<AMPCharacter>(ConfirmHitResult.GetActor());
			if (HitCharacter == nullptr || !HitCharacters.Contains(HitCharacter)) continue;
			
			if (ShotgunResult.BodyShots.Contains(HitCharacter)) ShotgunResult.BodyShots[HitCharacter]++;
			else ShotgunResult.BodyShots.Emplace(HitCharacter, 1);
		}
	}

	for (auto& CurrentFrame : CurrentFrames)
	{
		MoveBoxes(CurrentFrame.Character, CurrentFrame, true);
		EnableCharacterMeshCollision(CurrentFrame.Character, ECollisionEnabled::QueryAndPhysics);
	}

	return ShotgunResult;
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package,
	AMPCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity,
	float HitTime)
{
	return FServerSideRewindResult{true, true};
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(AMPCharacter* HitCharacter,
                                                                    const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(
	const TArray<AMPCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> FramesToCheck;
	for (AMPCharacter* HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}
	return ShotgunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(AMPCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	return FServerSideRewindResult();
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(AMPCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);
	if (HitCharacter == nullptr || Character == nullptr || Character->GetEquippedWeapon() == nullptr || !Confirm.bHitConfirmed) return;
	const float Damage = Character->GetEquippedWeapon()->GetDamage();
	UGameplayStatics::ApplyDamage(HitCharacter, Damage, Character->Controller, Character->GetEquippedWeapon(), UDamageType::StaticClass());
}

void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(
	const TArray<AMPCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	FShotgunServerSideRewindResult Confirm = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);
	if (Character == nullptr || Character->GetEquippedWeapon() == nullptr) return;

	for (auto& HitCharacter : HitCharacters)
	{
		const float HeadShotDamage = Confirm.HeadShots.Contains(HitCharacter)
			? Confirm.HeadShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetDamage()
			: 0.f;
		const float BodyShotDamage = Confirm.BodyShots.Contains(HitCharacter)
			? Confirm.BodyShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetDamage()
			: 0.f;
		const float TotalDamage = HeadShotDamage + BodyShotDamage;
		UGameplayStatics::ApplyDamage(HitCharacter, TotalDamage, Character->Controller, Character->GetEquippedWeapon(), UDamageType::StaticClass());
	}
}

void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(AMPCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
}