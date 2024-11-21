// Copyright Epic Games, Inc. All Rights Reserved.

#include "UE_MP_ShooterGameMode.h"
#include "UE_MP_ShooterCharacter.h"
#include "UObject/ConstructorHelpers.h"

AUE_MP_ShooterGameMode::AUE_MP_ShooterGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
