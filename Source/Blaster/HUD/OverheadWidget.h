// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

class UTextBlock;

UENUM()
enum class ENRole : uint8
{
	Remote,
	Local
};

UCLASS()
class BLASTER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn, ENRole NetRole) const;

	UFUNCTION(BlueprintCallable)
	void ShowPlayerName(APawn* InPawn) const;

protected:
	virtual void NativeDestruct() override;

private:
	void SetDisplayText(const FString& TextToDisplay) const;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> DisplayText;
};
