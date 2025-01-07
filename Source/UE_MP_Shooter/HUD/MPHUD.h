// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MPHUD.generated.h"

class UElimAnnouncement;
class UAnnouncement;
class UCharacterOverlay;

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	UTexture2D* CrosshairCenter;
	UTexture2D* CrosshairLeft;
	UTexture2D* CrosshairRight;
	UTexture2D* CrosshairTop;
	UTexture2D* CrosshairBottom;
	float CrosshairSpread;
	FLinearColor CrosshairColor;
};

/**
 * 
 */
UCLASS()
class UE_MP_SHOOTER_API AMPHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
	void AddCharacterOverlay();
	void AddAnnouncement();
	void AddElimAnnouncement(FString Attacker, FString Victim);
	
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;
	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	UPROPERTY(EditAnywhere, Category = "Announcement")
	TSubclassOf<UUserWidget> AnnouncementClass;
	UPROPERTY()
	UAnnouncement* Announcement;

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY()
	APlayerController* OwningPlayer;
	
	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2d Spread, FLinearColor Color);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UElimAnnouncement> ElimAnnouncementClass;
	UPROPERTY(EditAnywhere)
	float ElimAnnouncementTime = 2.5f;
	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove);
	UPROPERTY()
	TArray<UElimAnnouncement*> ElimMessages;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
