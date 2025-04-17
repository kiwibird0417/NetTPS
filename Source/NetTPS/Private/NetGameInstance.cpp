// Fill out your copyright notice in the Description page of Project Settings.


#include "NetGameInstance.h"
#include "../../../../Plugins/Online/OnlineSubsystem/Source/Public/Interfaces/OnlineSessionInterface.h"
#include "../../../../Plugins/Online/OnlineSubsystem/Source/Public/OnlineSessionSettings.h"
#include "NetTPS.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "../../../../Plugins/Online/OnlineBase/Source/Public/Online/OnlineSessionNames.h"



void UNetGameInstance::Init()
{
	Super::Init();

	if (auto subsys = IOnlineSubsystem::Get()) {
		
		// 서브 시스템으로부터 세션 인터페이스 가져오기
		sessionInterface = subsys->GetSessionInterface();

		sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnCreateSessionComplete);

		//----------------------------------------------------------------------
		//0415(화) 추가...
		sessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UNetGameInstance::OnFindSessionsComplete);

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
	
		// Session 검색
		//FTimerHandle handle;
		//GetWorld()->GetTimerManager().SetTimer(
		//	handle,
		//	FTimerDelegate::CreateLambda([&] 
		//		{
		//			FindOtherSession();
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
	sessionSettings.Set(FName("ROOM_NAME"), StringBase64Encode(roomName), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// 8. 호스트 네임 설정
	sessionSettings.Set(FName("HOST_NAME"), StringBase64Encode(mySessionName), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// netID
	FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();

	PRINTLOG(TEXT("Create Session Start : %s"), *mySessionName);
	sessionInterface->CreateSession(*netID, FName(mySessionName), sessionSettings);
	

}

void UNetGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	PRINTLOG(TEXT("SessionName : %s, bWasSuccessful : %d"), *SessionName.ToString(), bWasSuccessful);
}

//===================================================================================
//0415(화)
void UNetGameInstance::FindOtherSession()
{
	//시작하자마자 막기!
	onSearchState.Broadcast(true);


	//--------------------------------------------------------
	sessionSearch = MakeShareable(new FOnlineSessionSearch());

	// 1. 세션
	sessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	// 2. Lan 여부
	sessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL");

	// 3. 최대 검색 세션 수
	sessionSearch->MaxSearchResults = 10;

	// 4. 세션 검색
	sessionInterface->FindSessions(0, sessionSearch.ToSharedRef());

}


void UNetGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	// 찾기 실패 시
	if (bWasSuccessful == false) {
		
		onSearchState.Broadcast(false);

		PRINTLOG(TEXT("Session search failed"));
		return;
	}

	// 세션 검색 결과 배열
	auto results = sessionSearch->SearchResults;
	PRINTLOG(TEXT("Search Result Count : %d"), results.Num());

	for (int i = 0; i < results.Num(); i++) {
		auto sr = results[i];
		if (sr.IsValid() == false) {
			continue;
		}


		FString roomName;
		FString hostName;


		// 세션 정보 구조체 선언
		FSessionInfo sessionInfo;
		sessionInfo.index = i;

		//FString roomName;
		//sr.Session.SessionSettings.Get(FName("ROOM_NAME"), sessionInfo.roomName);
		sr.Session.SessionSettings.Get(FName("ROOM_NAME"), roomName);

		//FString hostName;
		//sr.Session.SessionSettings.Get(FName("HOST_NAME"), sessionInfo.hostName);
		sr.Session.SessionSettings.Get(FName("HOST_NAME"), hostName);
		sessionInfo.roomName = StringBase64Decode(roomName);
		sessionInfo.roomName = StringBase64Decode(hostName);

		// 세션 주인(방장) 이름
		FString userName = sr.Session.OwningUserName;

		// 입장 가능한 플레이어수
		int32 maxPlayerCount = sr.Session.SessionSettings.NumPublicConnections;

		// 현재 입장한 플레이어 수 ( 최대 - 현재 입장 가능한 수)
		// NumOpenPublicConnections : 스팀에서만 정상적으로 값이 들어온다
		int32 currentPlayerCount = maxPlayerCount - sr.Session.NumOpenPublicConnections;
		sessionInfo.playerCount = FString::Printf(TEXT("(%d/%d)"), currentPlayerCount, maxPlayerCount);

		// 핑 정보 ( 스팀에서는 9999로 나온다(기존), 바뀔 수 있음...)
		sessionInfo.pingSpeed = sr.PingInMs;


		PRINTLOG(TEXT("%s"), *sessionInfo.ToString());

		//--------------------------------------------------------------------
		//0417(목)
		//델리게이트로 위젯에 알려주기
		onSearchCompleted.Broadcast(sessionInfo);

	}

	// 더안전하게 막기 ( 프로세스 다 실행 이후, 버튼 막았던 거 취소하기)
	// 여러 번 반복해서 클릭하지 않게...
	onSearchState.Broadcast(false);

	/*
	// 정보를 가져온다
	for (auto sr : results) 
	{
		// 정보가 유효한지 체크
		if (sr.IsValid() == false) {
			continue;
		}

		FString roomName;
		sr.Session.SessionSettings.Get(FName("ROOM_NAME"), roomName);

		FString hostName;
		sr.Session.SessionSettings.Get(FName("HOST_NAME"), hostName);

		// 세션 주인(방장) 이름
		FString userName = sr.Session.OwningUserName;

		// 입장 가능한 플레이어수
		int32 maxPlayerCount = sr.Session.SessionSettings.NumPublicConnections;

		// 현재 입장한 플레이어 수 ( 최대 - 현재 입장 가능한 수)
		// NumOpenPublicConnections : 스팀에서만 정상적으로 값이 들어온다
		int32 currentPlayerCount = maxPlayerCount - sr.Session.NumOpenPublicConnections;

		// 핑 정보 ( 스팀에서는 9999로 나온다(기존), 바뀔 수 있음...)
		int32 pingSpeed = sr.PingInMs;

		PRINTLOG(TEXT("%s : %s(%s) - (%d/%d), %dms"), *roomName, *hostName, *userName, currentPlayerCount, maxPlayerCount, pingSpeed);
	}

	*/
}

//===================================================================================
// 다국어 지원(인코딩)

FString UNetGameInstance::StringBase64Encode(const FString& str)
{
	// Set할 때 : FString -> UTF8 (std::string) -> TArray<uint8> -> base64로 Encode
	std::string utf8String = TCHAR_TO_UTF8(*str);
	TArray<uint8> arrayData = TArray<uint8>((uint8*)utf8String.c_str(), utf8String.length());
	return FBase64::Encode(arrayData);
}

FString UNetGameInstance::StringBase64Decode(const FString& str)
{
	// Get할 때 : base64로 Decode -> TArray<uint8> -> TCHAR
	TArray<uint8> arrayData;
	FBase64::Decode(str, arrayData);
	std::string utf8String((char*)arrayData.GetData(), arrayData.Num());
	return UTF8_TO_TCHAR(utf8String.c_str());
}

/*
* 언리얼의 FString = TCHAR 배열
* TCHAR = UTF16(wchar_t, 2byte)
* 스팀 서버를 이용하면 깨진다.
* 원인은 명확히는 모르나, 아마도 UTF-8을 사용하여 발생하는 문제 같음.
* 
* 이런 문제를 해결하기 위해서 Base64 인코딩 / 디코딩을 이용
* 이걸 이용하는 이유는 안정하게 변환을 해서 전달이 가능
* Base64 인코딩 : 문자열을 uint8 배열로 만든 후
* ASCII 코드로 변환해서 사용
* 2의 6승 = 6bit 형식으로 인코딩 = 6bit씩 끊어서 인코딩
* 
* 
*/

//===================================================================================
