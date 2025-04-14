// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NetPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API ANetPlayerController : public APlayerController
{
	GENERATED_BODY()

//----------------------------------------------------
//0410(목)
private:
	UPROPERTY()
	class ANetTPSGameMode* gm;

public:
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void ServerRPC_RespawnPlayer();

	//----------------------------------------------------
	// 사용할 위젯클래스
	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<class UMainUI> mainUIWidget;

	// mainUIWidget으로 부터 만들어진 인스턴스
	UPROPERTY()
	class UMainUI* mainUI;

//-------------------------------------------------------
//0411(금)
// 플레이어가 죽은 후에, 다른 플레이어가 영혼이나 아이템 등 볼 수 있는 형태 만들기

	UFUNCTION(Server, Reliable)
	void ServerRPC_ChangeToSpectator();

	
};
