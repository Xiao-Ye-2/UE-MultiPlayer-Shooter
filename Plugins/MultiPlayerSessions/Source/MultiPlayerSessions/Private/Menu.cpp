// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiPlayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

void UMenu::MenuSetUp(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiPlayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiPlayerSessionsSubsystem>();
	}

	if (MultiPlayerSessionsSubsystem)
	{
		MultiPlayerSessionsSubsystem->MultiPlayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiPlayerSessionsSubsystem->MultiPlayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiPlayerSessionsSubsystem->MultiPlayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiPlayerSessionsSubsystem->MultiPlayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
		MultiPlayerSessionsSubsystem->MultiPlayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
	}
}

bool UMenu::Initialize()
{
	if (!Super::Initialize()) return false;

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}
	return true;
}

void UMenu::NativeDestruct()
{
	MenuTearDown();
	Super::NativeDestruct();
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(PathToLobby);
		}
	}
	else
	{
		HostButton->SetIsEnabled(true);
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSuccessful)
{
	if (MultiPlayerSessionsSubsystem == nullptr) return;
	for (auto Result : SessionResult)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		if (SettingsValue == MatchType)
		{
			MultiPlayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}
	if (!bWasSuccessful || SessionResult.Num() <= 0)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
	
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	if (!MultiPlayerSessionsSubsystem) return;

	MultiPlayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
}

void UMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if (!MultiPlayerSessionsSubsystem) return;

	MultiPlayerSessionsSubsystem->FindSessions(10000);
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputMode;
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}
