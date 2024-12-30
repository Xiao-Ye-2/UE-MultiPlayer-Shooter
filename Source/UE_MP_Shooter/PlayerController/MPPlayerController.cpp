// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UE_MP_Shooter/Character/MPCharacter.h"
#include "UE_MP_Shooter/GameMode/BlasterGameMode.h"
#include "UE_MP_Shooter/HUD/Announcement.h"
#include "UE_MP_Shooter/HUD/CharacterOverlay.h"
#include "UE_MP_Shooter/HUD/MPHUD.h"

void AMPPlayerController::BeginPlay()
{
	Super::BeginPlay();

	MPHUD = Cast<AMPHUD>(GetHUD());
	ServerCheckMatchState();
}

void AMPPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, MatchState);
}

void AMPPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	InitializeCharacterOverlay();
}

void AMPPlayerController::InitializeCharacterOverlay()
{
	if (CharacterOverlay == nullptr)
	{
		if (MPHUD && MPHUD->CharacterOverlay)
		{
			CharacterOverlay = MPHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
	}
}

void AMPPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncTime += DeltaTime;
	if (IsLocalController() && TimeSyncTime >= TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncTime -= TimeSyncFrequency;
	}
}

void AMPPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

float AMPPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AMPPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AMPPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
	float TimeServerReceivedClientRequest)
{
	float CurrentClientTime = GetWorld()->GetTimeSeconds();
	float RoundTripTime = CurrentClientTime - TimeOfClientRequest;
	float CurrentSeverTimeEstimate = TimeServerReceivedClientRequest + 0.5f * RoundTripTime;
	ClientServerDelta = CurrentSeverTimeEstimate - CurrentClientTime;
}

void AMPPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AMPCharacter* MPCharacter = Cast<AMPCharacter>(InPawn);
	if (MPCharacter)
	{
		SetHUDHealth(MPCharacter->GetHealth(), MPCharacter->GetMaxHealth());
	}
}

void AMPPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - (GetServerTime() - LevelStartingTime);
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - (GetServerTime() - LevelStartingTime);
	
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (CountDownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart)
		{
			SetHUDAnnouncementCountDown(TimeLeft);
		}
		else if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountDown(TimeLeft);
		}
		CountDownInt = SecondsLeft;
	}
}

void AMPPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
	if (MPHUD && MPHUD->CharacterOverlay && MPHUD->CharacterOverlay->HealthBar && MPHUD->CharacterOverlay->HealthText)
	{
		const float HealthPercent = Health / MaxHealth;
		MPHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		MPHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	} else
	{
		bInitializedCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AMPPlayerController::SetHUDScore(float Score)
{
	MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
	if (MPHUD && MPHUD->CharacterOverlay && MPHUD->CharacterOverlay->ScoreAmount)
	{
		MPHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(FString::FromInt(FMath::CeilToInt(Score))));
	} else
	{
		bInitializedCharacterOverlay = true;
		HUDScore = Score;
	}
}

void AMPPlayerController::SetHUDDefeats(int32 Defeats)
{
	MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
	if (MPHUD && MPHUD->CharacterOverlay && MPHUD->CharacterOverlay->DefeatsAmount)
	{
		MPHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(FString::FromInt(Defeats)));
	} else
	{
		bInitializedCharacterOverlay = true;
		HUDDefeats = Defeats;
	}
}

void AMPPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
	if (MPHUD && MPHUD->CharacterOverlay && MPHUD->CharacterOverlay->WeaponAmmoAmount)
	{
		MPHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(FString::FromInt(Ammo)));
	}
}

void AMPPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
	if (MPHUD && MPHUD->CharacterOverlay && MPHUD->CharacterOverlay->CarriedAmmoAmount)
	{
		MPHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(FString::FromInt(Ammo)));
	}
}

void AMPPlayerController::SetHUDMatchCountDown(float time)
{
	MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
	if (MPHUD && MPHUD->CharacterOverlay && MPHUD->CharacterOverlay->MatchCountDownText)
	{
		int32 Minutes = FMath::FloorToInt(time / 60.f);
		int32 Seconds = time - (Minutes * 60);
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		MPHUD->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(CountDownText));
	}
}


void AMPPlayerController::SetHUDAnnouncementCountDown(float time)
{
	MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
	if (MPHUD && MPHUD->Announcement && MPHUD->Announcement->WarmupTime)
	{
		int32 Minutes = FMath::FloorToInt(time / 60.f);
		int32 Seconds = time - (Minutes * 60);
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		MPHUD->Announcement->WarmupTime->SetText(FText::FromString(CountDownText));
	}
}

void AMPPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	OnRep_MatchState();
}

void AMPPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
		if (MPHUD == nullptr) return;
		MPHUD->AddCharacterOverlay();
		if (MPHUD->Announcement == nullptr) return;
		MPHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AMPPlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode == nullptr) return;
	ClientJoinMidGame(GameMode->GetMatchState(), GameMode->WarmupTime, GameMode->MatchTime, GameMode->LevelStaringTime);
}

void AMPPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match,
	float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);

	if (MPHUD && MatchState == MatchState::WaitingToStart)
	{
		MPHUD->AddAnnouncement();
	}
}