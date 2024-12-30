// Fill out your copyright notice in the Description page of Project Settings.


#include "MPGameState.h"

#include "Net/UnrealNetwork.h"
#include "UE_MP_Shooter/PlayerState/MPPlayerState.h"

void AMPGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, TopScoringPlayers);
}

void AMPGameState::UpdateTopScore(AMPPlayerState* PlayerState)
{
	if (TopScoringPlayers.Num() == 0 || PlayerState->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(PlayerState);
		TopScore = PlayerState->GetScore();
	} else if (PlayerState->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(PlayerState);
	}
}
