// Fill out your copyright notice in the Description page of Project Settings.

#include "MonsterDamageGameplayAbility.h"

UMonsterDamageGameplayAbility::UMonsterDamageGameplayAbility()
{
	// AI 폰은 소유 커넥션이 없어 부모의 LocalPredicted도 결과적으로는 서버 단독 실행이다.
	// 그래도 명시해 둔다 - 몬스터를 플레이어가 빙의하게 되면 그때부터 둘이 갈라진다.
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}
