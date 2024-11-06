// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
#pragma region Nullchecks
	if (!ElimmedCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|ElimmedCharacter is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	ElimmedCharacter->Elim();
}
