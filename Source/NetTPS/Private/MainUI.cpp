// Fill out your copyright notice in the Description page of Project Settings.


#include "MainUI.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NetPlayerController.h"

void UMainUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 1-1. 상호작용 버튼 연결
	if (btn_retry)
	{
		btn_retry->OnClicked.AddDynamic(this, &UMainUI::OnRetryButtonClicked);
	}

	//----------------------------------------------------------
	// 1-2. 플레이어 인벤토리 버튼 연결
	if (btn_exit)
	{
		btn_exit->OnClicked.AddDynamic(this, &UMainUI::OnExitButtonClicked);
	}

}

//------------------------------------------------------------------------------------
void UMainUI::ShowCrosshair(bool isShow)
{
	if( isShow )
	{
		img_Crosshair->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		img_Crosshair->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMainUI::AddBullet()
{
	auto BulletWidget = CreateWidget(GetWorld(), BulletUIFactory);
	BulletPanel->AddChildToUniformGrid(BulletWidget, 1, BulletPanel->GetChildrenCount());
}

void UMainUI::PopBullet(int32 index)
{
	BulletPanel->RemoveChildAt(index);
}

void UMainUI::RemoveAllAmmo()
{
	for( auto bulletWidget : BulletPanel->GetAllChildren() )
	{
		BulletPanel->RemoveChild(bulletWidget);
	}
}

void UMainUI::PlayDamageAnimation()
{
	PlayAnimation(DamageAnim);
}

void UMainUI::OnRetryButtonClicked()
{
	// 게임 종료 UI 안 보이도록 처리
	GameoverUI->SetVisibility(ESlateVisibility::Hidden);

	// 리스폰
	auto pc = Cast<ANetPlayerController>(GetWorld()->GetFirstPlayerController());
	if (pc) {
		// 마우스 커서 안 보이도록 처리
		pc->SetShowMouseCursor(false);

		// 리스폰 요청
		pc->ServerRPC_RespawnPlayer();
	}
}

void UMainUI::OnExitButtonClicked()
{

}

