﻿// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetTPSCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "NetPlayerAnimInstance.h"
#include "MainUI.h"
#include "Components/WidgetComponent.h"
#include "HealthBar.h"
#include "NetTPS.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraShakeBase.h"
#include "Components/HorizontalBox.h"
#include "NetPlayerController.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ANetTPSCharacter

ANetTPSCharacter::ANetTPSCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 150.0f; // The camera follows at this distance behind the character	
	CameraBoom->SetRelativeLocation( FVector( 0.0f, 40.0f, 60.0f ) );
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)


	GunComp = CreateDefaultSubobject<USceneComponent>(TEXT("GunComp"));
	GunComp->SetupAttachment(GetMesh(), TEXT("GunPosition"));	

	// Health bar component
	hpUIComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	hpUIComp->SetupAttachment(GetMesh());

}

void ANetTPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 만약 여기서 안 된다면!
	// Possessed 쪽 함수에서 불리도록 설정함.
	if (IsLocallyControlled() && HasAuthority() == false) {
		InitUIWidget();
	}

	//---------------------------------------------------
	// 총 검색
	TArray<AActor*> allActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), allActors);
	for( auto tempPistol : allActors )
	{
		if( tempPistol->GetName().Contains("BP_Pistol") )
		{
			pistolActors.Add(tempPistol);
		}
	}
}



//////////////////////////////////////////////////////////////////////////
// Input

void ANetTPSCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ANetTPSCharacter::TakePistol(const FInputActionValue& Value)
{
	// 총을 소유하고 있지 않다면 일정범위 안에 있는 총을 잡는다.

	// 필요속성 : 총소유여부, 소유중인 총, 총 검색범위

	// 1. 총을 잡고 있지 않다면
	if( bHasPistol == true )
	{
		return;
	}

	// 클라이언트에서 서버로 요청 ( 여기는 클라 )
	ServerRPC_TakePistol();	
}

void ANetTPSCharacter::AttachPistol(AActor* pistolActor)
{
	auto meshComp = pistolActor->GetComponentByClass<UStaticMeshComponent>();
	meshComp->SetSimulatePhysics(false);
	meshComp->AttachToComponent(GunComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	if( IsLocallyControlled() && mainUI )
	{
		mainUI->ShowCrosshair(true);
	}	
}

void ANetTPSCharacter::ReleasePistol(const FInputActionValue& Value)
{
	// 총을 잡고 있지 않거나 재장전 중이라면 처리하지 않는다
	if(bHasPistol == false || IsReloading || IsLocallyControlled() == false)
	{
		return;
	}

	ServerRPC_ReleasePistol();	
}

void ANetTPSCharacter::DetachPistol(AActor* pistolActor)
{
	auto meshComp = pistolActor->GetComponentByClass<UStaticMeshComponent>();
	meshComp->SetSimulatePhysics(true);
	meshComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);

	if( IsLocallyControlled() && mainUI )
	{
		mainUI->ShowCrosshair(false);
	}	
}

void ANetTPSCharacter::Fire(const FInputActionValue& Value)
{
	// 총을 들고 있지 않거나 재장전 중이거나 총알이 없을 경우 처리하지 않는다.
	if( !bHasPistol || IsReloading || BulletCount <= 0 )
	{
		return;
	}

	ServerRPC_Fire();

}

void ANetTPSCharacter::InitUIWidget()
{

	// 컨트롤러가 있는 로컬 플레이어라면 Player
	// 없으면 Not Player
	PRINTLOG(TEXT("[%s] Begin"), Controller ? TEXT("PLAYER") : TEXT("Not Player"));


	//---------------------------------------------------------
	// Player가 제어중이 아니라면 처리하지 않는다.
	auto pc = Cast<ANetPlayerController>(Controller);
	if( pc == nullptr )
	{
		return;
	}

	if( pc->mainUIWidget )
	{
		if (pc->mainUI == nullptr) {
			pc->mainUI = Cast<UMainUI>(CreateWidget(GetWorld(), pc->mainUIWidget));
		}

		mainUI = pc->mainUI;
		
		mainUI->AddToViewport();
		mainUI->ShowCrosshair(false);

		//================================================================
		// 다시 리스폰될 때, 최대 체력으로 다시 초기화
		// 리스폰 고려한 게임에서는 중요.
		hp = MaxHP;
		mainUI->HP = 1.0f;

		// 총알 모두 제거(Reload)
		mainUI->RemoveAllAmmo();

		//================================================================
		// 총알추가
		BulletCount = MaxBulletCount;


		for( int i = 0; i < MaxBulletCount; ++i )
		{
			mainUI->AddBullet();
		}

		// mainUI 가 있기 때문에 해당 컴포넌트는 비활성화
		if (hpUIComp) {
			hpUIComp->SetVisibility(false);
		}
	}
}

//============================================================================
// 이 함수는 서버에서만 호출된다.
void ANetTPSCharacter::PossessedBy(AController* NewController)
{
	PRINTLOG(TEXT("Begin"));

	Super::PossessedBy(NewController);

	// BeginPlay에서 InitUI 가 안 되었으면, 무조건 될 수 있도록...
	if (IsLocallyControlled()) {
		InitUIWidget();
	}

	PRINTLOG(TEXT("End"));


}

//============================================================================
void ANetTPSCharacter::ReloadPistol(const FInputActionValue& Value)
{
	// 총 소지중이 아니거나 재장전 중이라면 아무 처리하지 않는다.
	if( !bHasPistol || IsReloading )
	{
		return;
	}

	// 재장전 애니메이션 재생
	auto anim = Cast<UNetPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	anim->PlayReloadAnimation();	

	IsReloading = true;
}


void ANetTPSCharacter::InitAmmoUI()
{
	ServerRPC_Reload();
}

void ANetTPSCharacter::OnRep_HP()
{

	// 사망처리
	if (HP <= 0)
	{
		isDead = true;
		ReleasePistol(FInputActionValue());

		// 충돌체 없애주기
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCharacterMovement()->DisableMovement();
	}

	//===============================================

	// UI에 할당할 퍼센트 계산
	float percent = hp / MaxHP;

	if (mainUI)
	{
		mainUI->HP = percent;

		// 피격 효과 처리
		mainUI->PlayDamageAnimation();

		// 0410(목)
		// 카메라 셰이크 효과 재생
		if (damageCameraShake) {
			auto pc = Cast<APlayerController>(Controller);
			pc->ClientStartCameraShake(damageCameraShake);
		}
	}
	else
	{
		auto hpUI = Cast<UHealthBar>(hpUIComp->GetWidget());
		hpUI->HP = percent;
	}
}

float ANetTPSCharacter::GetHP()
{
	return hp;
}

void ANetTPSCharacter::SetHP(float value)
{
	hp = value;
	OnRep_HP();
}

void ANetTPSCharacter::DamageProcess()
{
	// 체력을 감소시킨다.
	HP--;

}


// 죽음 처리
void ANetTPSCharacter::DieProcess()
{	
	auto pc = Cast<APlayerController>(Controller);
	pc->SetShowMouseCursor(true);
	GetFollowCamera()->PostProcessSettings.ColorSaturation = FVector4(0.0f, 0.0f, 0.0f, 1.0f);

	// Die UI 표시
	if (mainUI) {
		mainUI->GameoverUI->SetVisibility(ESlateVisibility::Visible);
	}


}

//============================================================================
void ANetTPSCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//---------------------
	// HP Bar 빌보드
	if (hpUIComp && hpUIComp->GetVisibleFlag()) {
		FVector CamLoc = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation();

		FVector Direction = CamLoc - hpUIComp->GetComponentLocation();
		Direction.Z = 0.0f;

		hpUIComp->SetWorldRotation(Direction.GetSafeNormal().ToOrientationRotator());
	}



	//---------------------
	PrintNetLog();

}

void ANetTPSCharacter::PrintNetLog()
{
	const FString conStr = GetNetConnection() != nullptr ? TEXT("Valid Connection") : TEXT("Invalid Connection");

	const FString ownerName = GetOwner() != nullptr ? GetOwner()->GetName() : TEXT("No Owner");

	const FString logStr = FString::Printf(TEXT("Connection : %s\nOwner Name : %s\nLocal Role : %s\nRemote Role : %s"), *conStr, *ownerName, *LOCAL_ROLE, *REMOTE_ROLE);

	DrawDebugString(GetWorld(), GetActorLocation() + FVector::UpVector * 100.0f, logStr, nullptr, FColor::White, 0, true, 1);

	// 권한(Authority)
	// ROLE_Authority : 모든 권한을 다 가지고 있다. ( 로직 실행 가능 )
	// HasAuthority()
	
	// ROLE_AutonomouseProxy : 제어(Input)만 가능
	// IsLocallyControlled()

	// ROLE_SimulatedProxy : 시뮬레이션만 가능

}

void ANetTPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANetTPSCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANetTPSCharacter::Look);

		EnhancedInputComponent->BindAction(TakePistolAction, ETriggerEvent::Started, this, &ANetTPSCharacter::TakePistol);

		EnhancedInputComponent->BindAction(ReleaseAction, ETriggerEvent::Started, this, &ANetTPSCharacter::ReleasePistol);

		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ANetTPSCharacter::Fire);

		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ANetTPSCharacter::ReloadPistol);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ANetTPSCharacter::ServerRPC_TakePistol_Implementation()
{
	// 클라에서 요청받은 총잡기를 처리한다 ( 여기는 서버 )
	
	// 2. 월드에 있는 총을 모두 조사한다
	for( auto pistolActor : pistolActors )
	{
		// 3. 만약 총의 소유자가 있다면 검사하지 않는다.
		if( pistolActor->GetOwner() != nullptr )
		{
			continue;
		}

		// 4. 총과의 거리를 구한다.
		float Distance = FVector::Dist(GetActorLocation(), pistolActor->GetActorLocation());

		// 5. 거리가 범위 안에 있다면
		if( Distance > DistanceToGun )
		{
			continue;
		}

		// 6. 소유중인 총으로 등록
		ownedPistol = pistolActor;

		// 7. 총의 소유자를 자신으로 등록
		ownedPistol->SetOwner(this);

		// 8. 총 소유 상태로 변경
		bHasPistol = true;

		// 총을 잡으라고 클라에 요청 ( 여기는 서버 )
		MulticastRPC_TakePistol(pistolActor);		

		break;
	}
}

void ANetTPSCharacter::MulticastRPC_TakePistol_Implementation(AActor* pistolActor)
{
	// 서버에서 인자로 넘어온 총 액터를 붙이자 ( 여기는 클라 )
	// 총 붙이기
	AttachPistol(pistolActor);
}

void ANetTPSCharacter::ServerRPC_ReleasePistol_Implementation()
{
	// 총 소유시
	if( ownedPistol )
	{
		MulticastRPC_ReleasePistol(ownedPistol);		

		// 미소유로 설정
		bHasPistol = false;
		ownedPistol->SetOwner(nullptr);
		ownedPistol = nullptr;
	}
}

void ANetTPSCharacter::MulticastRPC_ReleasePistol_Implementation(AActor* pistolActor)
{
	// 총분리
	DetachPistol(pistolActor);
}

//-----------------------------------------------------------------------------------------
// 총쏘기
void ANetTPSCharacter::ServerRPC_Fire_Implementation()
{
	// 총알제거
	BulletCount--;

	// 총쏘기
	FHitResult hitInfo;
	FVector startPos = FollowCamera->GetComponentLocation();
	FVector endPos = startPos + FollowCamera->GetForwardVector() * 10000.0f;
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, startPos, endPos, ECollisionChannel::ECC_Visibility, params);

	if (bHit)
	{
		// 맞은 대상이 상대방일 경우 데미지 처리
		auto otherPlayer = Cast<ANetTPSCharacter>(hitInfo.GetActor());
		if (otherPlayer)
		{
			otherPlayer->DamageProcess();
		}
	}

	MulticastRPC_Fire(bHit, hitInfo);
}

void ANetTPSCharacter::MulticastRPC_Fire_Implementation(bool bHit, const FHitResult& hitInfo)
{
	if (mainUI) {
		mainUI->PopBullet(BulletCount);
	}

	// 총쏘기 애니메이션 재생
	auto anim = Cast<UNetPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	anim->PlayFireAnimation();

	if (bHit) {
		// 맞은 부위에 파티클 표시
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), GunEffect, hitInfo.Location, FRotator(), true);
	}

}

void ANetTPSCharacter::ServerRPC_Reload_Implementation()
{
	// 총알 개수 초기화
	BulletCount = MaxBulletCount;

	ClientRPC_Reload();
}

void ANetTPSCharacter::ClientRPC_Reload_Implementation()
{
	// 총알 UI 제거
	if (mainUI) {
		mainUI->RemoveAllAmmo();

		// 총알 UI 다시 세팅
		for (int i = 0; i < MaxBulletCount; ++i)
		{
			mainUI->AddBullet();
		}
	}

	// 재장전 완료상태로 처리
	IsReloading = false;
}


//-----------------------------------------------------------------------------------------
void ANetTPSCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ANetTPSCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ANetTPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetTPSCharacter, bHasPistol);	
	DOREPLIFETIME(ANetTPSCharacter, BulletCount);
	DOREPLIFETIME(ANetTPSCharacter, hp);

}