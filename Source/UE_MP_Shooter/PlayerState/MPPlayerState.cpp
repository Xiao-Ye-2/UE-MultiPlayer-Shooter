// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPlayerState.h"

#include "Net/UnrealNetwork.h"
#include "UE_MP_Shooter/Character/MPCharacter.h"
#include "UE_MP_Shooter/PlayerController/MPPlayerController.h"


void AMPPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(ScoreAmount + GetScore());
	UpdateScoreHUD();
}

void AMPPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	UpdateDefeatsHUD();
}

void AMPPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Defeats);
}

void AMPPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	UpdateScoreHUD();
}

void AMPPlayerState::OnRep_Defeats()
{
	UpdateDefeatsHUD();
}

void AMPPlayerState::UpdateScoreHUD()
{
	if (!IsControllerValid()) return;
	Controller->SetHUDScore(GetScore());
}

void AMPPlayerState::UpdateDefeatsHUD()
{
	if (!IsControllerValid()) return;
	Controller->SetHUDDefeats(Defeats);
}

bool AMPPlayerState::IsControllerValid()
{
	Character = Character == nullptr ? Cast<AMPCharacter>(GetPawn()) : Character;
	if (Character == nullptr) return false;
	Controller = Controller == nullptr ? Cast<AMPPlayerController>(Character->GetController()) : Controller;
	if (Controller == nullptr) return false;
	return true;
}
