// Fill out your copyright notice in the Description page of Project Settings.


#include "NetActor.h"
#include "NetTPS.h"
#include "NetTPSCharacter.h"
#include "EngineUtils.h"

// Sets default values
ANetActor::ANetActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Comp"));
	SetRootComponent(meshComp);
	meshComp->SetRelativeScale3D(FVector(0.5f));

}

// Called when the game starts or when spawned
void ANetActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	FindOwner();
	PrintNetLog();

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

