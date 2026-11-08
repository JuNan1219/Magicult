// Fill out your copyright notice in the Description page of Project Settings.

#include "MagicultDebug.h"

#include "HAL/IConsoleManager.h"

namespace
{
	TAutoConsoleVariable<bool> CVarMagicultDrawDebug(
		TEXT("Magicult.DrawDebug"),
		false,
		TEXT("전투/어빌리티 디버그 드로우 전역 토글. 0=끔, 1=켬"),
		ECVF_Cheat
	);
}

bool IsMagicultDrawDebugEnabled()
{
	return CVarMagicultDrawDebug.GetValueOnAnyThread();
}
