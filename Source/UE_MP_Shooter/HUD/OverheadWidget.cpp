// Fill out your copyright notice in the Description page of Project Settings.


#include "OverHeadWidget.h"
#include "Components/TextBlock.h"

void UOverheadWidget::SetDisplayText(FString text)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(text));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* Pawn)
{
	ENetRole RemoveRole = Pawn->GetRemoteRole();
	FString RoleName;
	switch (RemoveRole)
	{
	case ROLE_Authority:
		RoleName = FString("Authority");
		break;
	case ROLE_AutonomousProxy:
		RoleName = FString("Autonomous Proxy");
		break;
	case ROLE_SimulatedProxy:
		RoleName = FString("Simulated Proxy");
		break;
	default:
		RoleName = FString("None");
		break;
	}
	FString RemoveRoleString = FString::Printf(TEXT("%s"), *RoleName);
	SetDisplayText(RemoveRoleString);
}

void UOverheadWidget::NativeDestruct()
{
	Super::NativeDestruct();
	RemoveFromParent();
}
