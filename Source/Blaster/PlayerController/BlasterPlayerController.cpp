// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
}

void ABlasterPlayerController::SetHUDHealth(const float Health, const float MaxHealth) const
{
#pragma region Nullchecks
	if (!BlasterHUD)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|BlasterHUD is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!BlasterHUD->CharacterOverlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|BlasterHUD->CharacterOverlay is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!BlasterHUD->CharacterOverlay->HealthBar)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|BlasterHUD->CharacterOverlay->HealthBar is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!BlasterHUD->CharacterOverlay->HealthText)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|BlasterHUD->CharacterOverlay->HealthText is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	const float HealthPercent{Health / MaxHealth};
	BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
	const FString HealthText{FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth))};
	BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
}
