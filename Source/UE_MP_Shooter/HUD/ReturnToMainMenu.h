// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"

class UMultiPlayerSessionsSubsystem;
class APlayerController;
/**
 * 
 */
UCLASS()
class UE_MP_SHOOTER_API UReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	void MenuSetup();
	void MenuTearDown();

protected:
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnPlayerLeftGame();

private:
	UPROPERTY()
	APlayerController* PlayerController;
	
	UPROPERTY(meta = (BindWidget))
	class UButton* ReturnButton;
	
	UFUNCTION()
	void ReturnButtonClicked();
	
	UPROPERTY()
	UMultiPlayerSessionsSubsystem* MultiPlayerSessionsSubsystem;
};
