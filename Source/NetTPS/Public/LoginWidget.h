// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoginWidget.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API ULoginWidget : public UUserWidget
{
	GENERATED_BODY()

//===========================================================
public:
	virtual void NativeConstruct() override;	// 초기화

//===========================================================
//0414(월)
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(BindWidget))
	class UButton* btn_createRoom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	class UEditableText* edit_roomName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(BindWidget))
	class USlider* slider_playerCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock* txt_playerCount;
	
	//===========================================================
	UPROPERTY()
	class UNetGameInstance* gi;

	UFUNCTION()
	void CreateRoom();

	// Slider callback
	UFUNCTION()
	void OnValueChanged(float Value);


//===========================================================
//0416(수)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UWidgetSwitcher* WidgetSwitcher;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* btn_createSession;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* btn_findSession;


	UFUNCTION()
	void SwitchCreatePanel();

	UFUNCTION()
	void SwitchFindPanel();
	//===========================================================
	// 메인화면 돌아가기 버튼
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* btn_back;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* btn_back_1;
	

	UFUNCTION()
	void BacktoMain();

//===============================================================================
//0417(목)
	// 세션 슬롯
	// Canvas_FindRoom의 스크롤 박스 위젯
	UPROPERTY(BlueprintReadOnly, meta=(BindWIdget))
	class UScrollBox* scroll_roomList;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class USessionWidget> sessionInfoWidget;

	UFUNCTION()
	void AddSlotWidget(const struct FSessionInfo& sessionInfo);

	// 방 검색 버튼
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UButton* btn_find;

	// 방찾기 버튼 클릭시 호출될 콜백
	UFUNCTION()
	void OnClickedFindSession();


};
