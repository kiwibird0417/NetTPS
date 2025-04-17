// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionWidget.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API USessionWidget : public UUserWidget
{
	GENERATED_BODY()

//---------------------------------------------------
//0417(목)
public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* txt_roomName;	
	
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* txt_hostName;	
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* txt_playerCount;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* txt_pingSpeed;

	int32 sessionNumber;
	//---------------------------------------------------
	void Set(const struct FSessionInfo& sessionInfo);




//---------------------------------------------------

	
};
