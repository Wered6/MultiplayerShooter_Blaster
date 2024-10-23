// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn, ENRole NetRole) const
{
#pragma region Nullchecks
	if (!InPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|InPawn is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	ENetRole Role{};
	const ENetRole RemoteRole{InPawn->GetRemoteRole()};
	const ENetRole LocalRole{InPawn->GetLocalRole()};
	switch (NetRole)
	{
	case ENRole::Remote:
		Role = RemoteRole;
		break;
	case ENRole::Local:
		Role = LocalRole;
		break;
	}

	FString RoleString;
	switch (Role)
	{
	case ROLE_None:
		RoleString = FString("None");
		break;
	case ROLE_SimulatedProxy:
		RoleString = FString("SimulatedProxy");
		break;
	case ROLE_AutonomousProxy:
		RoleString = FString("AutonomousProxy");
		break;
	case ROLE_Authority:
		RoleString = FString("Authority");
		break;
	case ROLE_MAX:
		break;
	}

	switch (NetRole)
	{
	case ENRole::Remote:
		SetDisplayText(FString::Printf(TEXT("Remote Role: %s"), *RoleString));
		break;
	case ENRole::Local:
		SetDisplayText(FString::Printf(TEXT("Local Role: %s"), *RoleString));
		break;
	}
}

void UOverheadWidget::ShowPlayerName(APawn* InPawn) const
{
#pragma region Nullchecks
	if (!InPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|InPawn is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!InPawn->GetPlayerState())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|InPawn->GetPlayerState() is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	const FString PlayerName{InPawn->GetPlayerState()->GetPlayerName()};
	SetDisplayText(PlayerName);
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();

	Super::NativeDestruct();
}

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
