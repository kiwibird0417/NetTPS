// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionWidget.h"
#include "Components/TextBlock.h"
#include "NetGameInstance.h"

void USessionWidget::Set(const struct FSessionInfo& sessionInfo)
{
	if (txt_roomName) {
		txt_roomName->SetText(FText::FromString(sessionInfo.roomName));
	}

	if (txt_hostName) {
		txt_hostName->SetText(FText::FromString(sessionInfo.hostName));
	}

	if (txt_playerCount) {
		txt_playerCount->SetText(FText::FromString(sessionInfo.playerCount));
	}

	if (txt_pingSpeed)
	{
		FString pingText = FString::Printf(TEXT("%dms"), sessionInfo.pingSpeed);
		txt_pingSpeed->SetText(FText::FromString(pingText));
	}

	sessionNumber = sessionInfo.index;

}
