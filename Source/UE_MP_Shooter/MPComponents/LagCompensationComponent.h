#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class AMPPlayerController;
class AMPCharacter;

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;
	UPROPERTY()
	FRotator Rotation;
	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	TMap<FName, FBoxInformation> HitBoxInfo;
};


USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;
	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<AMPCharacter*, uint32> HeadShots;
	UPROPERTY()
	TMap<AMPCharacter*, uint32> BodyShots;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE_MP_SHOOTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend AMPCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color) const;

	/** 
	* Hitscan
	*/
	FServerSideRewindResult ServerSideRewind(AMPCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(AMPCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);

	/** 
	* Projectile
	*/
	FServerSideRewindResult ProjectileServerSideRewind(AMPCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);
	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(AMPCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);

	/** 
	* Shotgun
	*/
	FShotgunServerSideRewindResult ShotgunServerSideRewind(const TArray<AMPCharacter*>& HitCharacters,const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);
	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(const TArray<AMPCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);

protected:
	virtual void BeginPlay() override;
	void UpdateFramePackageHistory();
	void SaveFramePackage(FFramePackage& Package);
	void CacheBoxPositions(AMPCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(AMPCharacter* HitCharacter, const FFramePackage& Package, bool DisableCollision = false);
	void EnableCharacterMeshCollision(const AMPCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	FFramePackage GetFrameToCheck(AMPCharacter* HitCharacter, float HitTime);

	/** 
	* Hitscan
	*/
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, AMPCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);

	/** 
	* Projectile
	*/
	FServerSideRewindResult ProjectileConfirmHit(const FFramePackage& Package, AMPCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);

	/** 
	* Shotgun
	*/

	FShotgunServerSideRewindResult ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations);

private:
	UPROPERTY()
	AMPCharacter* Character;
	UPROPERTY()
	AMPPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;
	UPROPERTY()
	float MaxRecordTime = 1.5f;
public:	

		
};
