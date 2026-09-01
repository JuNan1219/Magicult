// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultGameInstance.h"
#include "MultiplayerSessionsSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "GameFramework/PlayerController.h"

void UMagicultGameInstance::Init()
{
	Super::Init();

	// 호스트 이탈/크래시로 클라이언트 연결이 끊기면 엔진이 발생시키는 실패 델리게이트.
	// 서버 GameMode의 Logout은 프로세스가 사라지면 클라까지 도달하지 못하므로 이쪽에서 잡는다.
	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UMagicultGameInstance::HandleNetworkFailure);
		GEngine->OnTravelFailure().AddUObject(this, &UMagicultGameInstance::HandleTravelFailure);
	}
}

void UMagicultGameInstance::Shutdown()
{
	if (GEngine)
	{
		GEngine->OnNetworkFailure().RemoveAll(this);
		GEngine->OnTravelFailure().RemoveAll(this);
	}

	Super::Shutdown();
}

void UMagicultGameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogTemp, Warning, TEXT("[Magicult] NetworkFailure: %s (%s)"),
		ENetworkFailure::ToString(FailureType), *ErrorString);

	// 서버(호스트) 자신은 여기서 메뉴로 튕겨선 안 된다. 클라이언트 연결(ServerConnection 보유)만 처리.
	// NetDriver가 없거나 ServerConnection이 없으면 우리가 서버이거나 이미 정리된 상태.
	if (!NetDriver || !NetDriver->ServerConnection)
	{
		return;
	}

	switch (FailureType)
	{
	case ENetworkFailure::ConnectionLost:
	case ENetworkFailure::ConnectionTimeout:
	case ENetworkFailure::FailureReceived:
	case ENetworkFailure::PendingConnectionFailure:
	case ENetworkFailure::NetDriverAlreadyExists:
	case ENetworkFailure::NetGuidMismatch:
	case ENetworkFailure::NetChecksumMismatch:
		TravelToMainMenu();
		break;
	default:
		break;
	}
}

void UMagicultGameInstance::HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogTemp, Warning, TEXT("[Magicult] TravelFailure: %s (%s)"),
		ETravelFailure::ToString(FailureType), *ErrorString);

	// 호스트가 사라진 뒤 트래블(맵 로드/접속)이 실패하는 경우에도 안전하게 메뉴로 복귀.
	TravelToMainMenu();
}

void UMagicultGameInstance::TravelToMainMenu()
{
	if (bReturningToMainMenu)
	{
		return;
	}
	bReturningToMainMenu = true;

	// 남아있는 세션은 정리해 두어야 다음 접속/호스팅이 깨끗하게 동작한다.
	if (UMultiplayerSessionsSubsystem* MSS = GetSubsystem<UMultiplayerSessionsSubsystem>())
	{
		MSS->DestroySession();
	}

	// ClientTravel로 로컬 플레이어를 메뉴 맵으로 이동(끊긴 연결을 정리하며 새 월드로 진입).
	if (APlayerController* PC = GetFirstLocalPlayerController())
	{
		PC->ClientTravel(MainMenuMap, TRAVEL_Absolute);
	}
	else if (UWorld* World = GetWorld())
	{
		// 컨트롤러가 이미 파괴된 극단적인 상황 대비.
		GEngine->SetClientTravel(World, *MainMenuMap, TRAVEL_Absolute);
	}

	// 트래블이 완료되면 새 월드에서 다시 실패를 받을 수 있도록 가드 해제(다음 프레임 이후).
	if (UWorld* World = GetWorld())
	{
		FTimerHandle Dummy;
		World->GetTimerManager().SetTimer(Dummy, [this]() { bReturningToMainMenu = false; }, 1.0f, false);
	}
	else
	{
		bReturningToMainMenu = false;
	}
}
