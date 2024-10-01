// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(const FString& TextToDisplay) const
{
#pragma region Nullchecks
	if (!DisplayText)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|DisplayText is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	DisplayText->SetText(FText::FromString(TextToDisplay));
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn) const
{
#pragma region Nullchecks
	if (!InPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|InPawn is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	const ENetRole RemoteRole{InPawn->GetRemoteRole()};
	const ENetRole LocalROle{InPawn->GetLocalRole()};

	FString Role;
	switch (RemoteRole)
	{
	case ROLE_None:
		Role = FString("None");
		break;
	case ROLE_SimulatedProxy:
		Role = FString("SimulatedProxy");
		break;
	case ROLE_AutonomousProxy:
		Role = FString("AutonomousProxy");
		break;
	case ROLE_Authority:
		Role = FString("Authority");
		break;
	case ROLE_MAX:
		break;
	}
	const FString RemoteRoleString{FString::Printf(TEXT("Remote Role: %s"), *Role)};
	FString PlayerName{InPawn->GetPlayerState()->GetPlayerName()};
	SetDisplayText(PlayerName);
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();

	Super::NativeDestruct();
}
