// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiPlayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"

UMultiPlayerSessionsSubsystem::UMultiPlayerSessionsSubsystem()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		SessionInterface = Subsystem->GetSessionInterface();
	}
}
