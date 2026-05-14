// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "SessionEntryWidget.generated.h"

class UTextBlock;
class UButton;

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API USessionEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    virtual bool Initialize() override;

    void SetSessionResult(const FOnlineSessionSearchResult& Result);

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> HostNameText;       // 호스트 이름

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> PlayerCountText;    // "2 / 4"

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> PingText;           // ms

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> SelectButton;

    FOnlineSessionSearchResult CachedResult;

    DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionEntrySelected, const FOnlineSessionSearchResult&);
    FOnSessionEntrySelected OnSelected;

private:
    UFUNCTION()
    void OnSelectClicked();
};
