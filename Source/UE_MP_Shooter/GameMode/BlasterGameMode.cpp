// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "UE_MP_Shooter/Character/MPCharacter.h"
#include "UE_MP_Shooter/GameState/MPGameState.h"
#include "UE_MP_Shooter/PlayerState/MPPlayerState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
}


void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStaringTime = GetWorld()->GetTimeSeconds();
}

void ABlasterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountDownTime = WarmupTime - (GetWorld()->GetTimeSeconds() - LevelStaringTime);
		if (CountDownTime <= 0.f)
		{
			StartMatch();
		}
	} else if (MatchState == MatchState::InProgress)
	{
		CountDownTime = WarmupTime + MatchTime - (GetWorld()->GetTimeSeconds() - LevelStaringTime);
		if (CountDownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	} else if (MatchState == MatchState::Cooldown)
	{
		CountDownTime = WarmupTime + MatchTime + CooldownTime - (GetWorld()->GetTimeSeconds() - LevelStaringTime);
		if (CountDownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AMPPlayerController* Controller = Cast<AMPPlayerController>(*It))
		{
			Controller->OnMatchStateSet(MatchState);
		}
	}
}

void ABlasterGameMode::PlayerEliminated(AMPCharacter* EliminatedCharacter,
                                        AMPPlayerController* EliminatedPlayerController,
                                        AMPPlayerController* AttakerPlayerController)
{
	AMPPlayerState* EliminatedPlayerState = EliminatedPlayerController? Cast<AMPPlayerState>(EliminatedPlayerController->PlayerState) : nullptr;
	AMPPlayerState* AttackerPlayerState = AttakerPlayerController ? Cast<AMPPlayerState>(AttakerPlayerController->PlayerState) : nullptr;
	AMPGameState* MPGameState = GetGameState<AMPGameState>();

	if (AttackerPlayerState && AttackerPlayerState != EliminatedPlayerState && MPGameState)
	{
		AttackerPlayerState->AddToScore(1.f);
		MPGameState->UpdateTopScore(AttackerPlayerState);
	}
	if (EliminatedPlayerState)
	{
		EliminatedPlayerState->AddToDefeats(1);
	}
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Eliminate();
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedPlayerController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}
	if (EliminatedPlayerController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(EliminatedPlayerController, PlayerStarts[Selection]);
	}
}