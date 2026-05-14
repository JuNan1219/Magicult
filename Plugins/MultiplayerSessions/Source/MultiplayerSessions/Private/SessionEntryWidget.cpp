// Fill out your copyright notice in the Description page of Project Settings.

#include "SessionEntryWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

bool USessionEntryWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (SelectButton)
	{
		SelectButton->OnClicked.AddDynamic(this, &ThisClass::OnSelectClicked);
	}

	return true;
}

void USessionEntryWidget::SetSessionResult(const FOnlineSessionSearchResult& Result)
{
	CachedResult = Result;

	if (HostNameText)
	{
		HostNameText->SetText(FText::FromString(Result.Session.OwningUserName));
	}

	if (PlayerCountText)
	{
		const int32 MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
		const int32 CurrentPlayers = MaxPlayers - Result.Session.NumOpenPublicConnections;
		PlayerCountText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentPlayers, MaxPlayers)));
	}

	if (PingText)
	{
		PingText->SetText(FText::FromString(FString::Printf(TEXT("%d ms"), Result.PingInMs)));
	}
}

void USessionEntryWidget::OnSelectClicked()
{
	OnSelected.Broadcast(CachedResult);
}
