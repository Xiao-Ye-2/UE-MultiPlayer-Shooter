// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "UE_MP_Shooter/Character/MPCharacter.h"
#include "UE_MP_Shooter/PlayerState/MPPlayerState.h"

void ABlasterGameMode::PlayerEliminated(AMPCharacter* EliminatedCharacter,
                                        AMPPlayerController* EliminatedPlayerController,
                                        AMPPlayerController* AttakerPlayerController)
{
	AMPPlayerState* EliminatedPlayerState = EliminatedPlayerController? Cast<AMPPlayerState>(EliminatedPlayerController->PlayerState) : nullptr;
	AMPPlayerState* AttackerPlayerState = AttakerPlayerController ? Cast<AMPPlayerState>(AttakerPlayerController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != EliminatedPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
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
