// Fill out your copyright notice in the Description page of Project Settings.


#include "NetGameInstance.h"
#include "../../../../Plugins/Online/OnlineSubsystem/Source/Public/Interfaces/OnlineSessionInterface.h"
#include "../../../../Plugins/Online/OnlineSubsystem/Source/Public/OnlineSessionSettings.h"
#include "NetTPS.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"

void UNetGameInstance::Init()
{
	Super::Init();

	if (auto subsys = IOnlineSubsystem::Get()) {
		
		// 서브 시스템으로부터 세션 인터페이스 가져오기
		sessionInterface = subsys->GetSessionInterface();

		sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnCreateSessionComplete);

		// 2초 뒤에 생성
		// 임시 테스트 코드
		//FTimerHandle handle;
		//GetWorld()->GetTimerManager().SetTimer(
		//	handle,
		//	FTimerDelegate::CreateLambda([&] 
		//		{
		//			CreateMySession(mySessionName, 10);
		//		}
		//	), 
		//	2.0f, false
		//);
	}
}

void UNetGameInstance::CreateMySession(FString roomName, int32 playerCount)
{
	// 세션 설정 변수
	FOnlineSessionSettings sessionSettings;

	// 1. Dedicated Server 접속 여부
	sessionSettings.bIsDedicated = false;

	// 2. 랜선(로컬) 매칭을 할지, Steam 매칭을 할 지 여부
	FName subsysName = IOnlineSubsystem::Get()->GetSubsystemName();
	sessionSettings.bIsLANMatch = (subsysName == "NULL");

	// 3. 매칭이 온라인을 통해 노출될지 여부
	// false일 때는 노출이 안 됨!
	// SendSessionInviteToFriend() 함수를 통해 친구초대를 할 수 있다
	sessionSettings.bShouldAdvertise = true;

	// 4. 온라인 상태(Presence) 정보를 활용할지 여부
	sessionSettings.bUsesPresence = true;

	// 버그? 
	sessionSettings.bUseLobbiesIfAvailable = true;

	// 5. 게임 진행중에 참여 허가할지 여부
	sessionSettings.bAllowJoinViaPresence = true;
	sessionSettings.bAllowJoinInProgress = true;

	// 6. 세션에 참여할 수 있는 공개(public) 연결의 최대 허용 수
	sessionSettings.NumPublicConnections = playerCount;

	// 7. 커스텀 룸네임 설정
	sessionSettings.Set(FName("ROOM_NAME"), roomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// 8. 호스트 네임 설정
	sessionSettings.Set(FName("HOST_NAME"), mySessionName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// netID
	FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();

	PRINTLOG(TEXT("Create Session Start : %s"), *mySessionName);
	sessionInterface->CreateSession(*netID, FName(mySessionName), sessionSettings);
	

}

void UNetGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	PRINTLOG(TEXT("SessionName : %s, bWasSuccessful : %d"), *SessionName.ToString(), bWasSuccessful);
}
