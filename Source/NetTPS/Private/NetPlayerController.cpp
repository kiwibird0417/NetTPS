// Fill out your copyright notice in the Description page of Project Settings.


#include "NetPlayerController.h"
#include "NetTPSGameMode.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/SpectatorPawn.h"

void ANetPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		gm = Cast<ANetTPSGameMode>(GetWorld()->GetAuthGameMode());

	}
}

void ANetPlayerController::ServerRPC_RespawnPlayer_Implementation()
{
	auto player = GetPawn();
	UnPossess();
	player->Destroy();

	gm->RestartPlayer(this);

}

//---------------------------------------------------------------------
//0411(금)
void ANetPlayerController::ServerRPC_ChangeToSpectator_Implementation()
{
	// 관전자가 플레이어의 위치에 생성될 수 있도록 플레이어 정보를 가져온다.
	APawn* player = GetPawn();

	if (player) 
	{
		// 관전자를 생성
		FActorSpawnParameters params;
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		auto spectator = GetWorld()->SpawnActor<ASpectatorPawn>(gm->SpectatorClass, player->GetActorTransform(), params);

		// 빙의 (Possess)
		Possess(spectator);

		// 이전 플레이어는 제거
		player->Destroy();

		// 5초 후에 리스폰 시키기
		FTimerHandle handle;

		// 서버 RPC를 부를 필요가 없을 때,
		// Implementation으로 부른다. (만일 Client였다면, 그냥 ServerRPC_RespawnPlayer를 호출했을 것)
		// 내부적으로 함수를 호출하듯이 사용!
		GetWorldTimerManager().SetTimer(handle, this, &ANetPlayerController::ServerRPC_RespawnPlayer_Implementation, 5.0f, false);

	}
}
