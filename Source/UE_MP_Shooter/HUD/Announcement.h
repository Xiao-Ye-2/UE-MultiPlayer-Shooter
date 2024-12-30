// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcement.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class UE_MP_SHOOTER_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(Meta = (BindWidget))
	UTextBlock* WarmupTime;

	UPROPERTY(Meta = (BindWidget))
	UTextBlock* AnnouncementText;

	UPROPERTY(Meta = (BindWidget))
	UTextBlock* InfoText;
};
