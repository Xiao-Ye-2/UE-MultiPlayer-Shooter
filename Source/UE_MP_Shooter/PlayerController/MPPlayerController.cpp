// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UE_MP_Shooter/Character/MPCharacter.h"
#include "UE_MP_Shooter/GameMode/BlasterGameMode.h"
#include "UE_MP_Shooter/GameState/MPGameState.h"
#include "UE_MP_Shooter/HUD/Announcement.h"
#include "UE_MP_Shooter/HUD/CharacterOverlay.h"
#include "UE_MP_Shooter/HUD/MPHUD.h"
#include "UE_MP_Shooter/MPComponents/CombatComponent.h"
#include "UE_MP_Shooter/PlayerState/MPPlayerState.h"

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
	if (CharacterOverlay != nullptr) return;
	
	if (MPHUD && MPHUD->CharacterOverlay)
	{
		CharacterOverlay = MPHUD->CharacterOverlay;
		if (CharacterOverlay == nullptr) return;
		
		if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
		if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
		if (bInitializeScore) SetHUDScore(HUDScore);
		if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
		if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);
		if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
		AMPCharacter* MPCharacter = Cast<AMPCharacter>(GetPawn());
		if (MPCharacter && MPCharacter->GetCombatComponent())
		{
			SetHUDGrenades(MPCharacter->GetCombatComponent()->GetGrenades());
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
	if (HasAuthority())
	{
		BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
		if (BlasterGameMode) TimeLeft = BlasterGameMode->GetCountDownTime();
	} else if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - (GetServerTime() - LevelStartingTime);
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - (GetServerTime() - LevelStartingTime);
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - (GetServerTime() - LevelStartingTime);
	
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (CountDownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
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
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AMPPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
	if (MPHUD && MPHUD->CharacterOverlay && MPHUD->CharacterOverlay->ShieldBar && MPHUD->CharacterOverlay->ShieldText)
	{
		const float ShieldPercent = Shield / MaxShield;
		MPHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		MPHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	} else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
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
		bInitializeScore = true;
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
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void AMPPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
	if (MPHUD && MPHUD->CharacterOverlay && MPHUD->CharacterOverlay->WeaponAmmoAmount)
	{
		MPHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(FString::FromInt(Ammo)));
	} else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void AMPPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
	if (MPHUD && MPHUD->CharacterOverlay && MPHUD->CharacterOverlay->CarriedAmmoAmount)
	{
		MPHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(FString::FromInt(Ammo)));
	} else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void AMPPlayerController::SetHUDMatchCountDown(float time)
{
	MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
	if (MPHUD && MPHUD->CharacterOverlay && MPHUD->CharacterOverlay->MatchCountDownText)
	{
		if (time < 0.f)
		{
			MPHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
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
		if (time < 0.f)
		{
			MPHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(time / 60.f);
		int32 Seconds = time - (Minutes * 60);
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		MPHUD->Announcement->WarmupTime->SetText(FText::FromString(CountDownText));
	}
}

void AMPPlayerController::SetHUDGrenades(uint32 grenades)
{
	MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
	if (MPHUD && MPHUD->CharacterOverlay && MPHUD->CharacterOverlay->GrenadesText)
	{
		MPHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(FString::FromInt(grenades)));
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
		HandleInProgress();
	} else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AMPPlayerController::HandleInProgress()
{
	MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
	if (MPHUD == nullptr) return;
	if (MPHUD->CharacterOverlay == nullptr) MPHUD->AddCharacterOverlay();
	if (MPHUD->Announcement == nullptr) return;
	MPHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
}

void AMPPlayerController::HandleCooldown()
{
	AMPCharacter* MPCharacter = Cast<AMPCharacter>(GetPawn());
	if (MPCharacter && MPCharacter->GetCombatComponent())
	{
		MPCharacter->bDisableGameplay = true;
		MPCharacter->GetCombatComponent()->FireButtonPressed(false);
	}
	
	MPHUD = MPHUD == nullptr ? Cast<AMPHUD>(GetHUD()) : MPHUD;
	if (MPHUD == nullptr) return;
	MPHUD->CharacterOverlay->RemoveFromParent();
	
	if (MPHUD->Announcement == nullptr
		|| MPHUD->Announcement->AnnouncementText == nullptr
		|| MPHUD->Announcement->InfoText == nullptr) return;
	MPHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
	FString AnnouncementText("New Match Starts In:");
	MPHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

	AMPGameState* MPGameState = Cast<AMPGameState>(UGameplayStatics::GetGameState(this));
	AMPPlayerState* MPPlayerState = GetPlayerState<AMPPlayerState>();
	FString InfoText;
	if (MPGameState && MPPlayerState)
	{
		TArray<AMPPlayerState*> TopPlayerStates = MPGameState->TopScoringPlayers;
		if (TopPlayerStates.Num() == 0)
		{
			InfoText = FString("There is no winner.");
		} else if (TopPlayerStates.Num() == 1 && TopPlayerStates[0] == MPPlayerState)
		{
			InfoText = FString("You won the game!");
		} else if (TopPlayerStates.Num() == 1)
		{
			InfoText = FString::Printf(TEXT("Winner: \n%s"), *TopPlayerStates[0]->GetPlayerName());
		} else
		{
			InfoText = FString("Players tied for the win:\n");
			for (auto TiedPlayer : TopPlayerStates) InfoText.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}
	MPHUD->Announcement->InfoText->SetText(FText::FromString(InfoText));
}

void AMPPlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode == nullptr) return;
	ClientJoinMidGame(GameMode->GetMatchState(), GameMode->WarmupTime, GameMode->MatchTime, GameMode->CooldownTime, GameMode->LevelStaringTime);
}

void AMPPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match,
	float Cooldown, float StartingTime)
{
	MatchState = StateOfMatch;
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	OnMatchStateSet(MatchState);

	if (MPHUD && MatchState == MatchState::WaitingToStart)
	{
		MPHUD->AddAnnouncement();
	}
}