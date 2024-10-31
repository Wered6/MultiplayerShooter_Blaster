// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

#include "CharacterOverlay.h"
#include "Blueprint/UserWidget.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();

	AddCharacterOverlay();
}

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

#pragma region Nullchecks
	if (!GEngine)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|GEngine is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	const float SpreadScaled{CrosshairSpreadMax * HUDPackage.CrosshairSpread};

	if (HUDPackage.CrosshairsCenter)
	{
		const FVector2D Spread(0.f);
		DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
	}
	if (HUDPackage.CrosshairsLeft)
	{
		const FVector2D Spread(-SpreadScaled, 0.f);
		DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
	}
	if (HUDPackage.CrosshairsRight)
	{
		const FVector2D Spread(SpreadScaled, 0.f);
		DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
	}
	if (HUDPackage.CrosshairsTop)
	{
		const FVector2D Spread(0.f, -SpreadScaled);
		DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
	}
	if (HUDPackage.CrosshairsBottom)
	{
		const FVector2D Spread(0.f, SpreadScaled);
		DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
	}
}

void ABlasterHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController{GetOwningPlayerController()};

#pragma region Nullchecks
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|PlayerController is nullptr"), *FString(__FUNCTION__))
		return;
	}
	if (!CharacterOverlayClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s|CharacterOverlayClass is nullptr"), *FString(__FUNCTION__))
		return;
	}
#pragma endregion

	CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
	CharacterOverlay->AddToViewport();
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, const FVector2D ViewportCenter, const FVector2D Spread, const FLinearColor CrosshairColor)
{
	const float TextureWidth{static_cast<float>(Texture->GetSizeX())};
	const float TextureHeight{static_cast<float>(Texture->GetSizeY())};
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - TextureWidth / 2.f + Spread.X,
		ViewportCenter.Y - TextureHeight / 2.f + Spread.Y
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairColor
	);
}
