// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameBase.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void ALobbyGameBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (GameState)
	{
		int32 NumPlayers = GameState.Get()->PlayerArray.Num();

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Yellow,
				FString::Printf(TEXT("Num Players: %d"), NumPlayers));

			APlayerState* NewPlayerState = NewPlayer->GetPlayerState<APlayerState>();
			if (NewPlayerState)
			{
				FString PlayerName = NewPlayerState->GetPlayerName();
				GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::Cyan,
		FString::Printf(TEXT("%s has joined the game!"), *PlayerName));
			}
		}
	}
}

void ALobbyGameBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	APlayerState* NewPlayerState = Exiting->GetPlayerState<APlayerState>();
	if (NewPlayerState && GEngine)
	{
		int32 NumPlayers = GameState.Get()->PlayerArray.Num();
		GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Yellow,
			FString::Printf(TEXT("Num Players: %d"), NumPlayers - 1));
		FString PlayerName = NewPlayerState->GetPlayerName();
		GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::Cyan,
FString::Printf(TEXT("%s has left the game!"), *PlayerName));
	}
}
