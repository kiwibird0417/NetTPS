// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerAnimInstance.h"
#include "NetTPSCharacter.h"
#include "MathUtil.h"

void UNetPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	player = Cast<ANetTPSCharacter>(TryGetPawnOwner()); 
}

void UNetPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if( player )
	{
		speed = FVector::DotProduct(player->GetVelocity(), player->GetActorForwardVector());

		direction = FVector::DotProduct(player->GetVelocity(), player->GetActorRightVector());

		// 회전값 적용
		pitchAngle = -player->GetBaseAimRotation().GetNormalized().Pitch;
		pitchAngle = FMathf::Clamp(pitchAngle, -60.0f, 60.0f);


		// 총소유 여부 적용
		bHasPistol = player->bHasPistol;
		// 사망 여부 적용
		isDead = player->isDead;
	}
}

void UNetPlayerAnimInstance::PlayFireAnimation()
{
	if( bHasPistol && FireMontage )
	{
		Montage_Play(FireMontage);
	}
}

void UNetPlayerAnimInstance::PlayReloadAnimation()
{
	if( bHasPistol && ReloadMontage )
	{
		Montage_Play(ReloadMontage);
	}
}

void UNetPlayerAnimInstance::AnimNotify_OnReloadFinish()
{
	player->InitAmmoUI();
}

void UNetPlayerAnimInstance::AnimNotify_DieEnd()
{
	player->DieProcess();
}


