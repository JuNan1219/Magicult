// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * MagiucultGameplayTags
 * 
 * Singleton contating natice Gameplay Tags
 */

struct FMagicultGameplayTags
{
public:
	static const FMagicultGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();

	// 친화도

	FGameplayTag Attributes_Affinity_EarthAffinity;
	FGameplayTag Attributes_Affinity_FireAffinity;
	FGameplayTag Attributes_Affinity_IceAffinity;
	FGameplayTag Attributes_Affinity_WindAffinity;
	FGameplayTag Attributes_Affinity_LightningAffinity;

	// 저항

	FGameplayTag Attributes_Resistance_EarthResistance;
	FGameplayTag Attributes_Resistance_FireResistance;
	FGameplayTag Attributes_Resistance_IceResistance;
	FGameplayTag Attributes_Resistance_WindResistance;
	FGameplayTag Attributes_Resistance_LightningResistance;

	// Vital

	FGameplayTag Attributes_Vital_Health;
	FGameplayTag Attributes_Vital_MaxHealth;
	FGameplayTag Attributes_Vital_Mana;
	FGameplayTag Attributes_Vital_MaxMana;
	FGameplayTag Attributes_Vital_MaxSpeed;

	// Input

	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_0;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;
	FGameplayTag InputTag_5;
	FGameplayTag InputTag_Dash;

	FGameplayTag InputTag_Detail_1;
	FGameplayTag InputTag_Detail_2;
	FGameplayTag InputTag_Detail_3;
	FGameplayTag InputTag_Detail_4;
	FGameplayTag InputTag_Detail_5;

	// 캐릭터

	FGameplayTag Character_State_MagicMode;
	FGameplayTag Character_State_Dead;
	FGameplayTag Character_State_Dashing;
	FGameplayTag Character_State_ModeInputBlocked;
	FGameplayTag Character_State_RecentlyDamaged;

	// 쿨다운 (GE가 부여 → CanActivateAbility가 존재 여부로 발동 차단)
	FGameplayTag Cooldown_Dash;

	// 피아식별 파벌. 같은 태그면 아군. BP 기본값에서 지정한다.
	FGameplayTag Character_Team_Player;
	FGameplayTag Character_Team_Enemy;

	FGameplayTag Character_Modifier_DamageModifier;

	// 캐릭터 입력 모드
	FGameplayTag Character_Mode;
	FGameplayTag Character_Mode_Direction;
	FGameplayTag Character_Mode_Speed;
	FGameplayTag Character_Mode_Element;
	FGameplayTag Character_Mode_AmmoType;
	FGameplayTag Character_Mode_Effect;

	// 마법
	FGameplayTag Magic;
	FGameplayTag Magic_None;

	FGameplayTag Magic_Speed_0;
	FGameplayTag Magic_Speed_1;
	FGameplayTag Magic_Speed_2;
	FGameplayTag Magic_Speed_3;
	FGameplayTag Magic_Speed_4; 

	FGameplayTag Magic_Direction_0;
	FGameplayTag Magic_Direction_1;
	FGameplayTag Magic_Direction_2;
	FGameplayTag Magic_Direction_3;
	FGameplayTag Magic_Direction_4;

	FGameplayTag Magic_Element_None;
	FGameplayTag Magic_Element_Fire;
	FGameplayTag Magic_Element_Ice;
	FGameplayTag Magic_Element_Earth;
	FGameplayTag Magic_Element_Wind;
	FGameplayTag Magic_Element_Lightning;

	//FGameplayTag Magic_AmmoType_Buckshot;
	FGameplayTag Magic_AmmoType_FastShot;
	FGameplayTag Magic_AmmoType_Explosive;
	FGameplayTag Magic_AmmoType_Penetrator;
	FGameplayTag Magic_AmmoType_Bounce;
	FGameplayTag Magic_AmmoType_Shield;

	// 액티브
	FGameplayTag Magic_Effect_SizeUp;
	FGameplayTag Magic_Effect_LongLife;
	FGameplayTag Magic_Effect_ZeroGravity;
	FGameplayTag Magic_Effect_Friendly;
	FGameplayTag Magic_Effect_Tracking;
	FGameplayTag Magic_Effect_DoubleCast;

	// 패시브
	FGameplayTag Magic_Effect_ProjectileGuide;
	FGameplayTag Magic_Effect_ManaGuard;
	FGameplayTag Magic_Effect_SuperArmor;
	FGameplayTag Magic_Effect_DefaultDirection;
	FGameplayTag Magic_Effect_DefaultSpeed;
	FGameplayTag Magic_Effect_Concentration;
	// Immunity 태그
	FGameplayTag Immunity_Damage;
	FGameplayTag Immunity_Knockback;
	FGameplayTag Immunity_Debuff;
	FGameplayTag Immunity_Debuff_Effect_Slow;
	FGameplayTag Immunity_Debuff_Effect_PeriodicDamage;
	FGameplayTag Immunity_Stun;

	// Action 태그
	FGameplayTag Action_Damage;
	FGameplayTag Action_RadialDamage;
	FGameplayTag Action_Knockback;
	FGameplayTag Action_Debuff;

	// Debuff 서브 이펙트 태그
	FGameplayTag Debuff_Effect_Slow;
	FGameplayTag Debuff_Effect_PeriodicDamage;

	// 상태
	FGameplayTag Magic_Status_Locked;
	FGameplayTag Magic_Status_Unlocked;
	FGameplayTag Magic_Status_Eligible;

	FGameplayTag Magic_Type_None;
	FGameplayTag Magic_Type_Active;
	FGameplayTag Magic_Type_Active_Projectile;
	FGameplayTag Magic_Type_Active_Beam;
	FGameplayTag Magic_Type_Active_Spray;
	FGameplayTag Magic_Type_Passive;

	// 상태이상
	FGameplayTag Aura_Fire;
	FGameplayTag Aura_Ice;
	FGameplayTag Aura_Earth;
	FGameplayTag Aura_Wind;
	FGameplayTag Aura_Lightning;

	FGameplayTag Debuff_Burn;
	FGameplayTag Debuff_Freeze;
	FGameplayTag Debuff_Stunned;
	FGameplayTag Debuff_Ragdoll;

	// 게임플레이 큐
	FGameplayTag GameplayCue_Fire;
	FGameplayTag GameplayCue_Ice;
	FGameplayTag GameplayCue_Earth;
	FGameplayTag GameplayCue_Wind;
	FGameplayTag GameplayCue_Lightning;

	FGameplayTag GameplayCue_Explosion;

	// 데이터
	FGameplayTag Data_Damage;
	FGameplayTag Data_DamageModifier;
	FGameplayTag Data_Modifier;
	FGameplayTag Data_Duration;
	FGameplayTag Data_TickInterval;

	// 노티파이 이벤트
	FGameplayTag Notify;
	FGameplayTag Notify_Attack;
	FGameplayTag Notify_Attack_Spawn;
	FGameplayTag Notify_Attack_Commit;
	FGameplayTag Notify_Attack_End;
	FGameplayTag Notify_Attack_HitboxOn;
	FGameplayTag Notify_Attack_HitboxOff;

	// 몬스터 공격

	FGameplayTag Monster_Attack_Melee;
	FGameplayTag Monster_Attack_Ranged;
	FGameplayTag Monster_Attack_Magic;
	FGameplayTag Monster_Attack_Special;
				 
	FGameplayTag Boss_Attack_Melee;
	FGameplayTag Boss_Attack_Ranged;
	FGameplayTag Boss_Attack_Magic;
	FGameplayTag Boss_Attack_Special;
	FGameplayTag Boss_Attack_Special_JumpSlam;
	FGameplayTag Boss_Attack_Special_Rush;

	// 몽타주 용
	FGameplayTag Effects_HitReact;

	// 트리거
	FGameplayTag Trigger_Hit;
	FGameplayTag Trigger_OnAttack;

	FGameplayTag Trigger_Projectile_Spawn;
	FGameplayTag Trigger_Projectile_SpawnFailure;
	FGameplayTag Trigger_Projectile_Expire;
	FGameplayTag Trigger_Projectile_Depleted;

	FGameplayTag AttackType_Direct;
	FGameplayTag AttackType_Radial;
	FGameplayTag HitType_Environment;

	// 소켓
	FGameplayTag CombatSocket_Weapon;
	FGameplayTag CombatSocket_LHand;
	FGameplayTag CombatSocket_RHand;
	FGameplayTag CombatSocket_Head;
	FGameplayTag CombatSocket_Magic;
	

	TArray<FGameplayTag> AllElements;
	TMap<FGameplayTag, FGameplayTag> ElementsToAura;
	TMap<FGameplayTag, FGameplayTag> ElementsToAffinity;
	TMap<FGameplayTag, FGameplayTag> ElementsToResistances;
	TMap<FGameplayTag, FGameplayTag> ElementsToGameplayCues;
private:
	static FMagicultGameplayTags GameplayTags;
};