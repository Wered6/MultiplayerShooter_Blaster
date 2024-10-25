// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UTexture2D> CrosshairsCenter;
	UPROPERTY()
	TObjectPtr<UTexture2D> CrosshairsLeft;
	UPROPERTY()
	TObjectPtr<UTexture2D> CrosshairsRight;
	UPROPERTY()
	TObjectPtr<UTexture2D> CrosshairsTop;
	UPROPERTY()
	TObjectPtr<UTexture2D> CrosshairsBottom;
};

UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package)
	{
		HUDPackage = Package;
	}

private:
	FHUDPackage HUDPackage;
};
