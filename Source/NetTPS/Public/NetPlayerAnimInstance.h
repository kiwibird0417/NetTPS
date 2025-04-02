// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "NetPlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class NETTPS_API UNetPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyAnimSettings")
	bool bHasPistol = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyAnimSettings")
	float direction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyAnimSettings")
	float speed;

	UPROPERTY()
	class ANetTPSCharacter* player;

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation( float DeltaSeconds ) override;
};
