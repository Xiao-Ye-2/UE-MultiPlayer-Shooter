#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

class APickup;

UCLASS()
class UE_MP_SHOOTER_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<APickup>> PickupClasses;

	UPROPERTY()
	APickup* SpawnedPickup;

	void SpawnPickup();
	UFUNCTION()
	void StartSpawnTimer(AActor* DestroyedActor);
	void SpawnTimerFinished();

private:
	FTimerHandle SpawnTimer;
	UPROPERTY(EditAnywhere)
	float SpawnTimeMin;
	UPROPERTY(EditAnywhere)
	float SpawnTimeMax;
public:	
	

};
