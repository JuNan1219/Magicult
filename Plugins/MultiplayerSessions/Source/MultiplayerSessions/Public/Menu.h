// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

class UMultiplayerSessionsSubsystem;
class UButton;
class UScrollBox;
class UTextBlock;
class USessionEntryWidget;

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnection = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("/Game/Maps/Lobby")));

protected:

	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	// 
	// MultiplayerSessionsSubsystem의 커스텀 델리게이트를 위한 콜백
	//
	UFUNCTION()
	void OnCreateSession(bool bWasSSuccessful);
	void OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSSuccessful);

private:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HostButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> SessionListBox;       // 방 목록 스크롤박스

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusText;           // "검색 중...", "세션 없음" 등

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<USessionEntryWidget> SessionEntryClass;  // BP에서 지정

	void OnSessionEntrySelected(const FOnlineSessionSearchResult& Result);

	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	void MenuTearDown();

	// 모든 온라인 세션 기능을 처리하도록 만들어진 서브시스템(GameInstance가 부모)
	UPROPERTY()
	TObjectPtr<UMultiplayerSessionsSubsystem> MultiplayerSessionsSubsystem;

	int32 NumPublicConnections{4};
	FString MatchType{TEXT("FreeForAll")};
	FString PathToLobby{ TEXT("") };
};
