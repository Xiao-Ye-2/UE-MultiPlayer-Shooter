
#include "PickupSpawnPoint.h"

#include "Pickup.h"

APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	StartSpawnTimer(nullptr);
}

void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickupSpawnPoint::SpawnPickup()
{
	int32 NumPickupClasses = PickupClasses.Num();
	if (NumPickupClasses == 0) return;
	int Index = FMath::RandRange(0, NumPickupClasses - 1);
	SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[Index], GetActorTransform());
	if (HasAuthority() && SpawnedPickup) SpawnedPickup->OnDestroyed.AddDynamic(this, &ThisClass::StartSpawnTimer);
}

void APickupSpawnPoint::StartSpawnTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::FRandRange(SpawnTimeMin, SpawnTimeMax);
	GetWorldTimerManager().SetTimer(SpawnTimer, this, &ThisClass::SpawnTimerFinished, SpawnTime);
}

void APickupSpawnPoint::SpawnTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickup();
	}
}
