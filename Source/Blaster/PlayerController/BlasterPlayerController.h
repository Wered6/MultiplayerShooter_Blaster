// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

class ABlasterHUD;

UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	void SetHUDHealth(const float Health, const float MaxHealth) const;

private:
	UPROPERTY()
	TObjectPtr<ABlasterHUD> BlasterHUD;
};
