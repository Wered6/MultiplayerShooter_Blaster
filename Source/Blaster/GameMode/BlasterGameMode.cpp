// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

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

void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
#pragma region Nullchecks
	if (!ElimmedCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|ElimmedCharacter is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!ElimmedController)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|ElimmedController is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	ElimmedCharacter->Reset();
	ElimmedCharacter->Destroy();

	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
	const int32 Selection{FMath::RandRange(0, PlayerStarts.Num() - 1)};
	RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
}
