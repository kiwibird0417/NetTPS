// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "NetTPSCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ANetTPSCharacter : public ACharacter
{
	GENERATED_BODY()

	// 총을 자식으로 붙일 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* GunComp;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* TakePistolAction;

	// 필요속성 : 총 소유여부
	bool bHasPistol = false;

	// 소유중인 총
	UPROPERTY()
	AActor* ownedPistol = nullptr;

	// 총 검색 범위
	UPROPERTY(EditAnywhere, Category = Gun)
	float DistanceToGun = 200.0f;

	// 월드에 배치된 총들
	UPROPERTY()
	TArray<AActor*> pistolActors;


	void TakePistol(const FInputActionValue& Value);

	// 총을 컴포넌트에 붙이기
	void AttachPistol(AActor* pistolActor);


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ReleaseAction;

	// 총 놓기입력 처리 함수
	void ReleasePistol(const FInputActionValue& Value);

	// 총을 컴포넌트에서 분리
	void DetachPistol(AActor* pistolActor);



	// 총 쏘기 입력액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* FireAction;

	// 총 쏘기 처리 함수
	void Fire(const FInputActionValue& Value);

	// 피격 파티클
	UPROPERTY(EditDefaultsOnly, Category = Gun)
	class UParticleSystem* GunEffect;



	// 사용할 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<class UMainUI> mainUIWidget;

	// mainUIWidget으로 부터 만들어진 인스턴스
	UPROPERTY()
	class UMainUI* mainUI;

	// UI 초기화 함수
	void InitUIWidget();



public:
	ANetTPSCharacter();
	
	virtual void BeginPlay() override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			

protected:

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

