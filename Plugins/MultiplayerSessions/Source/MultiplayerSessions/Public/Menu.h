// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

class UMultiplayerSessionsSubsystem;
class UButton;

UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4,
	               FString TypeOfMatch = FString(TEXT("FreeForAll")),
	               FString LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")));

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	/*
	 * Callbacks fot the custom delegates on the MultiplayerSessionSubsystem
	 */
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:
	UFUNCTION()
	void HostButtonClicked();
	UFUNCTION()
	void JoinButtonClicked();

	void MenuTearDown();

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> HostButton;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> JoinButton;

	// Subsystem designed to handle all online session functionality
	TObjectPtr<UMultiplayerSessionsSubsystem> MultiplayerSessionsSubsystem;

	int32 NumPublicConnections{4};
	FString MatchType{TEXT("FreeForAll")};
	UPROPERTY()
	FString PathToLobby{TEXT("")};
};
