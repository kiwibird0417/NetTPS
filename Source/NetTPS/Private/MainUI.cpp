// Fill out your copyright notice in the Description page of Project Settings.


#include "MainUI.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"

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
