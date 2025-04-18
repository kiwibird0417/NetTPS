// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "../../../../Plugins/Online/OnlineSubsystem/Source/Public/Interfaces/OnlineSessionInterface.h"
#include "Delegates/DelegateCombinations.h"
#include "NetGameInstance.generated.h"

USTRUCT(BlueprintType) 
struct FSessionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString roomName;

	UPROPERTY(BlueprintReadOnly)
	FString hostName;

	UPROPERTY(BlueprintReadOnly)
	FString playerCount;

	UPROPERTY(BlueprintReadOnly)
	int32 pingSpeed;

	UPROPERTY(BlueprintReadOnly)
	int32 index;

	inline FString ToString()
	{
		return FString::Printf(
			TEXT("[%d] %s : %s%s - %dms"),
			index,
			*roomName,
			*hostName,
			*playerCount,
			pingSpeed
		);
	}
};

// 세션 검색 끝났을 때 호출될 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSearchSignature, const FSessionInfo&, sessionInfo);

// 세션 검색 상태 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSearchStateSignature, bool, bIsSearching);

/**
 * 
 */
UCLASS()
class NETTPS_API UNetGameInstance : public UGameInstance
{
	GENERATED_BODY()

//=======================================================
public:
	virtual void Init() override;


//=======================================================
//0414(월)
public:
	IOnlineSessionPtr sessionInterface;

	void CreateMySession(FString roomName, int32 playerCount);

	// 세션(호스트) 이름
	FString mySessionName = "kiwibird0417";

	UFUNCTION()
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

//=======================================================
//0415(화)
	// 방 검색
	TSharedPtr<FOnlineSessionSearch> sessionSearch;
	void FindOtherSession();

	void OnFindSessionsComplete(bool bWasSuccessful);

	// 다국어 인코딩
	FString StringBase64Encode(const FString& str);
	FString StringBase64Decode(const FString& str);


//=======================================================
//0417(목)
	// 방찾기 완료 콜백을 등록할 델리게이트
	FSearchSignature onSearchCompleted;

	// 방 찾기 상태 콜백 델리게이트
	FSearchStateSignature onSearchState;

//=======================================================
//0418(금)
	// 세션(방) 입장
	void JoinSelectedSession(int32 index);
	
	// 세션 입장 콜백
	void OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result);


};
