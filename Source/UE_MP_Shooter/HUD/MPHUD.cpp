// Fill out your copyright notice in the Description page of Project Settings.

#include <vector>

#include "MPHUD.h"

#include "Announcement.h"
#include "CharacterOverlay.h"
#include "ElimAnnouncement.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"

void AMPHUD::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMPHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void AMPHUD::AddAnnouncement()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && AnnouncementClass)
	{
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		Announcement->AddToViewport();
	}
}

void AMPHUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
	OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;
	if (OwningPlayer == nullptr || ElimAnnouncementClass == nullptr) return;
	
	UElimAnnouncement* ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwningPlayer, ElimAnnouncementClass);
	if (ElimAnnouncementWidget == nullptr) return;
	
	ElimAnnouncementWidget->SetElimAnnouncementText(Attacker, Victim);
	ElimAnnouncementWidget->AddToViewport();

	for (auto Msg : ElimMessages)
	{
		if (Msg == nullptr || Msg->AnnouncementBox == nullptr) return;
		UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
		if (CanvasSlot == nullptr) continue;
		
		FVector2D Position = CanvasSlot->GetPosition();
		FVector2D NewPosition(CanvasSlot->GetPosition().X,Position.Y - CanvasSlot->GetSize().Y);
		CanvasSlot->SetPosition(NewPosition);
	}

	ElimMessages.Add(ElimAnnouncementWidget);

	FTimerHandle ElimMsgTimer;
	FTimerDelegate ElimMsgDelegate;
	ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncementWidget);
	GetWorldTimerManager().SetTimer(ElimMsgTimer, ElimMsgDelegate, ElimAnnouncementTime,false);
}

void AMPHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{
	if (MsgToRemove == nullptr) return;
	MsgToRemove->RemoveFromParent();
}


void AMPHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2d ViewportCenter = FVector2D(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
		
		std::vector<std::pair<UTexture2D*, FVector2D>> Crosshairs = {
			{HUDPackage.CrosshairCenter, FVector2D(0.f, 0.f)},
			{HUDPackage.CrosshairLeft, FVector2D(-SpreadScaled, 0.f)},
			{HUDPackage.CrosshairRight, FVector2D(SpreadScaled, 0.f)},
			{HUDPackage.CrosshairTop, FVector2D(0.f, -SpreadScaled)},
			{HUDPackage.CrosshairBottom, FVector2D(0.f, SpreadScaled)}
		};

		// Iterate over the crosshairs and draw them
		for (const auto& [Crosshair, Spread] : Crosshairs)
		{
			if (Crosshair)
			{
				DrawCrosshair(Crosshair, ViewportCenter, Spread, HUDPackage.CrosshairColor);
			}
		}
	}
}

void AMPHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2d Spread, FLinearColor Color)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2d DrawPoint = ViewportCenter - FVector2D(TextureWidth / 2.f, TextureHeight / 2.f) + Spread;

	DrawTexture(Texture, DrawPoint.X, DrawPoint.Y, TextureWidth, TextureHeight,
		0.f, 0.f, 1.f, 1.f,
		Color);
}
