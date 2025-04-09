// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainUI.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API UMainUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = UI, meta=(BindWidget))
	class UImage*	img_Crosshair;
	
	// 크로스헤어 보일지여부 처리
	void ShowCrosshair(bool isShow);


	// 총알 위젯이 추가될 패널
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UUniformGridPanel* BulletPanel;

	// 총알 위젯클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Bullet)
	TSubclassOf<class UUserWidget> BulletUIFactory;

	// 총알위젯 추가
	void AddBullet();

	// 총알제거
	void PopBullet(int32 index);

	// 모든 총알UI 제거
	void RemoveAllAmmo();


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = HP)
	float HP = 1.0f;

//-----------------------------------------------------------------------
//0409(수)
	//DamageUI 애니메이션
	UPROPERTY(EditDefaultsOnly, meta = (BindWidgetAnim), Transient, Category = "MySettings")
	class UWidgetAnimation* DamageAnim;

	// 피격 처리 애니메이션 재생
	void PlayDamageAnimation();




};
