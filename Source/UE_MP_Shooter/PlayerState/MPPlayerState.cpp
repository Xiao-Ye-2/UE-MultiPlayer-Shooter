// Fill out your copyright notice in the Description page of Project Settings.


#include "MPPlayerState.h"

#include "UE_MP_Shooter/Character/MPCharacter.h"
#include "UE_MP_Shooter/PlayerController/MPPlayerController.h"


void AMPPlayerState::AddToScore(float ScoreAmount)
{
	Score += ScoreAmount;
	UpdateScoreHUD();
}

void AMPPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	UpdateScoreHUD();
}

void AMPPlayerState::UpdateScoreHUD()
{
	Character = Character == nullptr ? Cast<AMPCharacter>(GetPawn()) : Character;
	if (Character == nullptr) return;
	Controller = Controller == nullptr ? Cast<AMPPlayerController>(Character->GetController()) : Controller;
	if (Controller == nullptr) return;

	Controller->SetHUDScore(Score);
}