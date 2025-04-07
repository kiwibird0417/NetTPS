// Fill out your copyright notice in the Description page of Project Settings.


#include "NetActor.h"
#include "NetTPS.h"
#include "NetTPSCharacter.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ANetActor::ANetActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Comp"));
	SetRootComponent(meshComp);
	meshComp->SetRelativeScale3D(FVector(0.5f));

	bReplicates = true;

	// 대역폭조정
	NetUpdateFrequency = 100.0f;
}

// Called when the game starts or when spawned
void ANetActor::BeginPlay()
{
	Super::BeginPlay();

	Mat = meshComp->CreateDynamicMaterialInstance(0);

	if( HasAuthority() )
	{
		FTimerHandle handle;

		GetWorldTimerManager().SetTimer(handle,
		FTimerDelegate::CreateLambda([&]
									{
										MatColor = FLinearColor(FMath::RandRange(0.0f, 0.3f),
																FMath::RandRange(0.0f, 0.3f),
																FMath::RandRange(0.0f, 0.3f),
																1.0f);

										OnRep_ChangeMatColor();
									}
								), 1, true );
	}	
}

// Called every frame
void ANetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	FindOwner();
	PrintNetLog();

	// Server 일 경우
	if( HasAuthority() )
	{
		// 서버 영역
		AddActorLocalRotation(FRotator(0.0f, 50.0f * DeltaTime, 0.0f));
		RotYaw = GetActorRotation().Yaw;
	}
	else
	{
		// 클라이언트 자체 보간
		// 경과시간 증가
		currentTime += DeltaTime;
		// 0으로 나눠지지 않도록 lastTime 값 체크
		if( lastTime < KINDA_SMALL_NUMBER )
		{
			return;
		}

		// 이전 경과시간과 현재 경과시간의 비율계산
		float lerpRatio = currentTime / lastTime;
		// 이전 경과시간 만큼 회전할 것으로 새로운 회전값 계산
		float newYaw = RotYaw + 50 * lastTime;
		// 예측되는 값으로 진행된 시간만큼 보관처리
		float lerpYaw = FMath::Lerp(RotYaw, newYaw, lerpRatio);
		// 최종 적용
		FRotator CurRot = GetActorRotation();
		CurRot.Yaw = lerpYaw;
		SetActorRotation(CurRot);
	}

	//else
	//{
	//	// 클라이언트 영역
	//	FRotator NewRot = GetActorRotation();
	//	NewRot.Yaw = RotYaw;
	//	SetActorRotation(NewRot);
	//}
}

void ANetActor::PrintNetLog()
{
	const FString conStr = GetNetConnection() != nullptr ? TEXT("Valid Connection") : TEXT("Invalid Connection");

	const FString ownerName = GetOwner() != nullptr ? GetOwner()->GetName() : TEXT("No Owner");

	const FString logStr = FString::Printf(TEXT("Connection : %s\nOwner Name : %s\nLocal Role : %s\nRemote Role : %s"), *conStr, *ownerName, *LOCAL_ROLE, *REMOTE_ROLE);

	DrawDebugString(GetWorld(), GetActorLocation() + FVector::UpVector * 100.0f, logStr, nullptr, FColor::Red, 0, true, 1);
}

void ANetActor::FindOwner()
{
	if( HasAuthority() )
	{
		AActor* newOwner = nullptr;
		float minDist = searchDistance;

		for( TActorIterator<ANetTPSCharacter> it(GetWorld()); it; ++it )
		{
			AActor* otherActor = *it;
			float dist = GetDistanceTo(otherActor);

			if( dist < minDist )
			{
				minDist = dist;
				newOwner = otherActor;
			}
		}

		// Owner 설정
		if( GetOwner() != newOwner )
		{
			SetOwner(newOwner);
		}
	}

	DrawDebugSphere(GetWorld(), GetActorLocation(), searchDistance, 30, FColor::Yellow, false, 0, 0, 1);
}

void ANetActor::OnRep_RotYaw()
{
	// 클라이언트 영역
	FRotator NewRot = GetActorRotation();
	NewRot.Yaw = RotYaw;
	SetActorRotation(NewRot);

	// 업데이트된 경과시간 저장
	lastTime = currentTime;

	// 경과시간을 초기화
	currentTime = 0.0f;
}

// 색상동기화
void ANetActor::OnRep_ChangeMatColor()
{
	if( Mat )
	{
		Mat->SetVectorParameterValue(TEXT("FloorColor"), MatColor);
	}
}

void ANetActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetActor, RotYaw);
	DOREPLIFETIME(ANetActor, MatColor);
}