// Fill out your copyright notice in the Description page of Project Settings.


#include "LoginWidget.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "NetGameInstance.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ScrollBox.h"
#include "SessionWidget.h"

void ULoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	gi = Cast<UNetGameInstance>(GetWorld()->GetGameInstance());

	//0417(목)
	// broadcast로 delegate를 통해 호출 받으면, 실행이 되는 형식이다.
	// NetGameInstance 쪽을 살펴보면 됨! Good~
	gi->onSearchCompleted.AddDynamic(this, &ULoginWidget::AddSlotWidget);

	//--------------------------------------------------------------------------
	btn_createRoom->OnClicked.AddDynamic(this, &ULoginWidget::CreateRoom);
	slider_playerCount->OnValueChanged.AddDynamic(this, &ULoginWidget::OnValueChanged);

	btn_createSession->OnClicked.AddDynamic(this, &ULoginWidget::SwitchCreatePanel);
	btn_findSession->OnClicked.AddDynamic(this, &ULoginWidget::SwitchFindPanel);

	btn_back->OnClicked.AddDynamic(this, &ULoginWidget::BacktoMain);
	btn_back_1->OnClicked.AddDynamic(this, &ULoginWidget::BacktoMain);

	//--------------------------------------------------------------------------
	btn_find->OnClicked.AddDynamic(this, &ULoginWidget::OnClickedFindSession);


}

void ULoginWidget::CreateRoom()
{
	if (gi && (edit_roomName->GetText().IsEmpty() == false)) {
		FString roomName = edit_roomName->GetText().ToString();
		int32 playerCount = slider_playerCount->GetValue();

		gi->CreateMySession(roomName, playerCount);
	}

}

void ULoginWidget::OnValueChanged(float Value)
{
	txt_playerCount->SetText(FText::AsNumber(Value));
}



//------------------------------------------

void ULoginWidget::SwitchCreatePanel()
{
	WidgetSwitcher->SetActiveWidgetIndex(1);
}

void ULoginWidget::SwitchFindPanel()
{
	WidgetSwitcher->SetActiveWidgetIndex(2);
}

void ULoginWidget::BacktoMain()
{
	WidgetSwitcher->SetActiveWidgetIndex(0);
}

//------------------------------------------
//0417(목)
void ULoginWidget::AddSlotWidget(const struct FSessionInfo& sessionInfo)
{
	auto slot = CreateWidget<USessionWidget>(this, sessionInfoWidget);
	slot->Set(sessionInfo);

	scroll_roomList->AddChild(slot);
}

void ULoginWidget::OnClickedFindSession()
{
	// 기존 슬롯이 있다면 모두 지운다
	scroll_roomList->ClearChildren();

	if (gi != nullptr) {
		gi->FindOtherSession();
	}
}

//------------------------------------------

