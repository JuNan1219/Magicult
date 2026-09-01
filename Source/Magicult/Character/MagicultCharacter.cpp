// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultCharacter.h"
#include "Magicult/Game/MagicultGameModeBase.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Magicult/Player/MagicultPlayerState.h"
#include "Magicult/UI/HUD/MagicultHUD.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemComponent.h"
#include "Magicult/MagicultGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "HAL/IConsoleManager.h"
#include "TimerManager.h"

// 테스트 중 재컴파일 없이 부활 대기시간을 바꾸기 위한 콘솔변수. 음수면 캐릭터의 RespawnDelay 값을 쓴다.
// 타이머는 서버에서만 도므로 리슨 서버/데디케이티드 서버 쪽에서 설정해야 적용된다.
static float GTestRespawnDelayOverride = -1.f;
static FAutoConsoleVariableRef CVarTestRespawnDelay(
	TEXT("magicult.TestRespawn.Delay"),
	GTestRespawnDelayOverride,
	TEXT("테스트 부활 대기시간(초). 음수면 캐릭터의 RespawnDelay 값을 사용한다."),
	ECVF_Default
);

AMagicultCharacter::AMagicultCharacter()
{
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
}

void AMagicultCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMagicultCharacter, PlayerIndex);
}

void AMagicultCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AMagicultCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// AGameModeBase는 SeamlessTravel 시 OverrideWith를 호출하지 않으므로
	// PlayerState 대신 PlayerArray 순서로 PlayerIndex 결정
	if (AGameStateBase* GS = GetWorld()->GetGameState())
	{
		if (APlayerState* PS = GetPlayerState<APlayerState>())
		{
			for (int32 i = 0; i < GS->PlayerArray.Num(); ++i)
			{
				if (GS->PlayerArray[i].Get() == PS)
				{
					PlayerIndex = i;
					break;
				}
			}
		}
	}
	ApplyPlayerColor();

	InitAbilityActorInfo();
	AddCharacterAbilities();

	// 속성 초기화(InitAbilityActorInfo→InitializeStartupData) 이후 서버에서 리젠 GE 적용
	ApplyHealthRegenEffect();
}

void AMagicultCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 클라이언트: Init ability actor info
	InitAbilityActorInfo();
}

void AMagicultCharacter::OnRep_PlayerIndex()
{
	ApplyPlayerColor();
}

void AMagicultCharacter::ApplyPlayerColor()
{
	UMaterialInterface* Material = (PlayerIndex == 0) ? Player1Material : Player2Material;
	if (Material && GetMesh())
	{
		GetMesh()->SetMaterial(0, Material);
	}
}

int32 AMagicultCharacter::GetPlayerLevel_Implementation()
{
	const AMagicultPlayerState* MagicultPlayerState = GetPlayerState<AMagicultPlayerState>();
	check(MagicultPlayerState);
	return MagicultPlayerState->GetPlayerLevel();
}

FName AMagicultCharacter::GetDefaultKnockbackBone() const
{
	return TEXT("Head_01");
}

void AMagicultCharacter::Die()
{
	// 부모의 사망 게이트(권한 없음 / 이미 사망)에 걸린 호출은 아래 후처리도 타면 안 되므로
	// 호출 전후의 bDead를 비교해 "이번 호출이 실제로 죽인 경우"만 걸러낸다.
	const bool bWasDead = bDead;

	Super::Die();

	if (!HasAuthority() || bWasDead || !bDead) return;

	// 테스트용 부활 예약. 실제 복구는 AMagicultCharacterBase::Revive()가 담당한다.
	if (bTestRespawnEnabled && !GetWorldTimerManager().IsTimerActive(RespawnTimerHandle))
	{
		GetWorldTimerManager().SetTimer(
			RespawnTimerHandle,
			this,
			&AMagicultCharacter::OnRespawnTimerFinished,
			GetEffectiveRespawnDelay(),
			false
		);
	}

	// 전멸 판정은 부활 예약 뒤에 알린다. 부활이 걸린 플레이어는 전멸에서 제외되므로 순서가 결과를 바꾼다.
	if (AMagicultGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AMagicultGameModeBase>())
	{
		GameMode->NotifyPlayerDied(this);
	}
}

bool AMagicultCharacter::IsAwaitingRespawn() const
{
	return GetWorldTimerManager().IsTimerActive(RespawnTimerHandle);
}

void AMagicultCharacter::CancelPendingRespawn()
{
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
}

float AMagicultCharacter::GetEffectiveRespawnDelay() const
{
	const float Delay = (GTestRespawnDelayOverride >= 0.f) ? GTestRespawnDelayOverride : RespawnDelay;
	// 타이머는 0 이하를 허용하지 않으므로 최소값을 준다.
	return FMath::Max(Delay, KINDA_SMALL_NUMBER);
}

void AMagicultCharacter::OnRespawnTimerFinished()
{
	Revive();
}

void AMagicultCharacter::RestoreDefaultMaterials()
{
	Super::RestoreDefaultMaterials();

	// 디졸브로 교체된 메시 머티리얼을 플레이어 색상으로 되돌린다.
	ApplyPlayerColor();
}

void AMagicultCharacter::InitAbilityActorInfo()
{
	AMagicultPlayerState* MagicultPlayerState = GetPlayerState<AMagicultPlayerState>();
	check(MagicultPlayerState);
	MagicultPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(MagicultPlayerState, this);
	Cast<UMagicultAbilitySystemComponent>(MagicultPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	AbilitySystemComponent = MagicultPlayerState->GetAbilitySystemComponent();
	AttributeSet = MagicultPlayerState->GetAttributeSet();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (AMagicultHUD* HUD = PlayerController->GetHUD<AMagicultHUD>())
		{
			HUD->InitOverlay(PlayerController, MagicultPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}
	InitializeDefaultAttributes();

	Super::InitAbilityActorInfo();
}

void AMagicultCharacter::InitializeDefaultAttributes() const
{
	if (AMagicultPlayerState* MagicultPlayerState = GetPlayerState<AMagicultPlayerState>())
	{
		MagicultPlayerState->InitializeStartupData();
	}
}
