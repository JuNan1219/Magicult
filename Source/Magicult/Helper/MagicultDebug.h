// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// 디버그 드로우 전역 스위치. 콘솔에서 `Magicult.DrawDebug 1`로 켠다.
// 각 액터/태스크의 bDrawDebug와 AND로 묶여, 이게 꺼져 있으면 아무것도 그려지지 않는다.
MAGICULT_API bool IsMagicultDrawDebugEnabled();
