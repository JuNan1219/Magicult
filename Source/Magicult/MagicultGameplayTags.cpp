// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultGameplayTags.h"
#include "GameplayTagsManager.h"

FMagicultGameplayTags FMagicultGameplayTags::GameplayTags;

void FMagicultGameplayTags::InitializeNativeGameplayTags()
{

	// Affinity Attributes
	GameplayTags.Attributes_Affinity_FireAffinity = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Affinity.FireAffinity"),
		FString("")
	);

	GameplayTags.Attributes_Affinity_IceAffinity = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Affinity.IceAffinity"),
		FString("")
	);

	GameplayTags.Attributes_Affinity_EarthAffinity = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Affinity.EarthAffinity"), 
		FString("")
	);
	
	GameplayTags.Attributes_Affinity_WindAffinity = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Affinity.WindAffinity"), 
		FString("")
	);
	GameplayTags.Attributes_Affinity_LightningAffinity = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Affinity.LightningAffinity"),
		FString("")
	);

	// Resistance Attributes
	GameplayTags.Attributes_Resistance_FireResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.FlameResistance"),
		FString("Reduces the time of flame condition")
	);
	GameplayTags.Attributes_Resistance_IceResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.IceResistance"),
		FString("")
	);
	GameplayTags.Attributes_Resistance_EarthResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.EarthResistance"), 
		FString("")
	);
	GameplayTags.Attributes_Resistance_WindResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.WindResistance"), 
		FString("")
	);
	GameplayTags.Attributes_Resistance_LightningResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.LightningResistance"),
		FString("")
	);

	// Vital Attributes
	GameplayTags.Attributes_Vital_Health = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.Health"),
		FString("")
	);
	GameplayTags.Attributes_Vital_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.MaxHealth"),
		FString("")
	);
	GameplayTags.Attributes_Vital_Mana = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.Mana"),
		FString("")
	);
	GameplayTags.Attributes_Vital_MaxMana = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.MaxMana"),
		FString("")
	);
	GameplayTags.Attributes_Vital_MaxSpeed = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.MaxSpeed"),
		FString("")
	);

	// Input Tag
	GameplayTags.InputTag_LMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.LMB"),
		TEXT("마우스 LMB")
	);

	GameplayTags.InputTag_RMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.RMB"),
		TEXT("마우스 RMB")
	);

	GameplayTags.InputTag_0 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.0"),
		TEXT("키보드 숫자 0")
	);

	GameplayTags.InputTag_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.1"),
		TEXT("키보드 숫자 1")
	);

	GameplayTags.InputTag_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.2"),
		TEXT("키보드 숫자 2")
	);

	GameplayTags.InputTag_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.3"),
		TEXT("키보드 숫자 3")
	);

	GameplayTags.InputTag_4 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.4"),
		TEXT("키보드 숫자 4")
	);

	GameplayTags.InputTag_5 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.5"),
		TEXT("키보드 숫자 5")
	);

	// Magic Detail Tag
	GameplayTags.InputTag_Detail_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.Detail.1"),
		TEXT("키보드 숫자 1")
	);

	GameplayTags.InputTag_Detail_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.Detail.2"),
		TEXT("키보드 숫자 2")
	);

	GameplayTags.InputTag_Detail_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.Detail.3"),
		TEXT("키보드 숫자 3")
	);

	GameplayTags.InputTag_Detail_4 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.Detail.4"),
		TEXT("키보드 숫자 4")
	);

	GameplayTags.InputTag_Detail_5 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.Detail.5"),
		TEXT("키보드 숫자 5")
	);


	// Charcter State
	GameplayTags.Character_State_MagicMode = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Character.State.MagicMode"),
		TEXT("")
	);
	GameplayTags.Character_State_Dead = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Character.State.Dead"),
		TEXT("")
	);

	GameplayTags.Character_Modifier_DamageModifier = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Character.Modifier.DamageModifier"),
		TEXT("")
	);

	// Character Input Mode
	GameplayTags.Character_Mode = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Character.Mode"),
		TEXT("")
	);

	GameplayTags.Character_Mode_Direction = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Character.Mode.Direction"),
		TEXT("")
	);
	GameplayTags.Character_Mode_Speed = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Character.Mode.Speed"),
		TEXT("")
	);
	GameplayTags.Character_Mode_Element = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Character.Mode.Element"),
		TEXT("")
	);
	GameplayTags.Character_Mode_AmmoType = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Character.Mode.AmmoType"),
		TEXT("")
	);
	GameplayTags.Character_Mode_Effect = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Character.Mode.Effect"),
		TEXT("")
	);
	// Magic
	GameplayTags.Magic = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic"),
		TEXT("")
	);

	GameplayTags.Magic_None = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.None"),
		TEXT("Magic이 설정되지 않았습니다.")
	);

	// Magic Speed

	GameplayTags.Magic_Speed_0 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Speed.0"),
		TEXT("발사체 속도 0")
	);

	GameplayTags.Magic_Speed_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Speed.1"),
		TEXT("발사체 속도 200")
	);

	GameplayTags.Magic_Speed_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Speed.2"),
		TEXT("발사체 속도 400")
	);

	GameplayTags.Magic_Speed_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Speed.3"),
		TEXT("발사체 속도 600")
	);

	GameplayTags.Magic_Speed_4 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Speed.4"),
		TEXT("발사체 속도 800")
	);

	// Magic Direction

	GameplayTags.Magic_Direction_0 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Direction.0"),
		TEXT("발사체 각도 0")
	);

	GameplayTags.Magic_Direction_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Direction.1"),
		TEXT("발사체 각도 15")
	);

	GameplayTags.Magic_Direction_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Direction.2"),
		TEXT("발사체 각도 30")
	);

	GameplayTags.Magic_Direction_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Direction.3"),
		TEXT("발사체 각도 45")
	);

	GameplayTags.Magic_Direction_4 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Direction.4"),
		TEXT("발사체 각도 60")
	);

	// Magic Element
	GameplayTags.Magic_Element_None = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Element.None"),
		TEXT("")
	);

	GameplayTags.Magic_Element_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Element.Fire"),
		TEXT("")
	);

	GameplayTags.Magic_Element_Ice = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Element.Ice"),
		TEXT("")
	);

	GameplayTags.Magic_Element_Earth = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Element.Earth"),
		TEXT("")
	);
	
	GameplayTags.Magic_Element_Wind = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Element.Wind"),
		TEXT("")
	);

	GameplayTags.Magic_Element_Lightning = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Element.Lightning"),
		TEXT("")
	);

	// Magic AmmoType
	GameplayTags.Magic_AmmoType_FastShot = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.AmmoType.FastShot"),
		TEXT("")
	);
	GameplayTags.Magic_AmmoType_Explosive = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.AmmoType.Explosive"),
		TEXT("")
	);
	GameplayTags.Magic_AmmoType_Penetrator = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.AmmoType.Penetrator"),
		TEXT("")
	);
	GameplayTags.Magic_AmmoType_Bounce = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.AmmoType.Bounce"),
		TEXT("")
	);
	GameplayTags.Magic_AmmoType_Shield = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.AmmoType.Shield"),
		TEXT("")
	);

	// Magic Effect - Active
	GameplayTags.Magic_Effect_SizeUp = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Effect.SizeUp"),
		TEXT("")
	);
	
	GameplayTags.Magic_Effect_LongLife = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Effect.LongLife"),
		TEXT("")
	);

	GameplayTags.Magic_Effect_ZeroGravity = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Effect.ZeroGravity"),
		TEXT("")
	);

	GameplayTags.Magic_Effect_Friendly = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Effect.Friendly"),
		TEXT("")
	);

	GameplayTags.Magic_Effect_Tracking = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Effect.Tracking"),
		TEXT("")
	);

	GameplayTags.Magic_Effect_DoubleCast = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Effect.DoubleCast"),
		TEXT("")
	);

	// Magic Effect - Passive

	GameplayTags.Magic_Effect_ProjectileGuide = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Effect.ProjectileGuide"),
		TEXT("")
	);

	GameplayTags.Magic_Effect_ManaGuard = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Effect.ManaGuard"),
		TEXT("")
	);

	GameplayTags.Magic_Effect_SuperArmor = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Effect.SuperArmor"),
		TEXT("")
	);

	GameplayTags.Magic_Effect_DefaultDirection = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Effect.DefaultDirection"),
		TEXT("")
	);

	GameplayTags.Magic_Effect_DefaultSpeed = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Effect.DefaultSpeed"),
		TEXT("")
	);

	GameplayTags.Magic_Effect_Concentration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Effect.Concentration"),
		TEXT("마법 모드 취소를 방지하는 집중 상태")
	);


	// Magic Status

	GameplayTags.Magic_Status_Locked = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Status.Locked"),
		TEXT("")
	);
	GameplayTags.Magic_Status_Unlocked = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Status.Unlocked"),
		TEXT("")
	);
	GameplayTags.Magic_Status_Eligible = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Status.Eligible"),
		TEXT("")
	);


	// Magic Type

	GameplayTags.Magic_Type_None = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Type.None"),
		TEXT("")
	);
	GameplayTags.Magic_Type_Active = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Type.Active"),
		TEXT("")
	);
	GameplayTags.Magic_Type_Active_Projectile = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Type.Active.Projectile"),
		TEXT("")
	);
	GameplayTags.Magic_Type_Active_Beam = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Type.Active.Beam"),
		TEXT("")
	);
	GameplayTags.Magic_Type_Active_Spray = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Type.Active.Spray"),
		TEXT("")
	);
	GameplayTags.Magic_Type_Passive = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Magic.Type.Passive"),
		TEXT("")
	);

	// Aura
	GameplayTags.Aura_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Aura.Fire"),
		TEXT("")
	);
	GameplayTags.Aura_Ice = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Aura.Ice"),
		TEXT("")
	);
	GameplayTags.Aura_Earth = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Aura.Earth"),
		TEXT("")
	);
	GameplayTags.Aura_Wind = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Aura.Wind"),
		TEXT("")
	);
	GameplayTags.Aura_Lightning = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Aura.Lightning"),
		TEXT("")
	);

	// Debuff

	GameplayTags.Debuff_Burn = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Debuff.Burn"),
		TEXT("")
	);
	GameplayTags.Debuff_Freeze = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Debuff.Freeze"),
		TEXT("")
	);
	GameplayTags.Debuff_Stunned = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Debuff.Stunned"),
		TEXT("")
	);
	GameplayTags.Debuff_Ragdoll = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Debuff.Ragdoll"),
		TEXT("")
	);

	// Gameplay Cue
	GameplayTags.GameplayCue_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("GameplayCue.Fire"),
		TEXT("")
	);
	GameplayTags.GameplayCue_Ice = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("GameplayCue.Ice"),
		TEXT("")
	);
	GameplayTags.GameplayCue_Earth = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("GameplayCue.Earth"),
		TEXT("")
	);
	GameplayTags.GameplayCue_Wind = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("GameplayCue.Wind"),
		TEXT("")
	);
	GameplayTags.GameplayCue_Lightning = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("GameplayCue.Lightning"),
		TEXT("")
	);

	GameplayTags.GameplayCue_Explosion = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("GameplayCue.Explosion"),
		TEXT("")
	);

	// Data
	GameplayTags.Data_Damage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Data.Damage"),
		TEXT("")
	);
	GameplayTags.Data_DamageModifier = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Data.DamageModifier"),
		TEXT("")
	);
	GameplayTags.Data_Modifier = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Data.Modifier"),
		TEXT("")
	);
	GameplayTags.Data_Duration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Data.Duration"),
		TEXT("")
	);
	GameplayTags.Data_TickInterval = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Data.TickInterval"),
		TEXT("")
	);

	// Montage Notify Event
	GameplayTags.Notify = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Notify"),
		TEXT("")
	);
	GameplayTags.Notify_Attack = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Notify.Attack"),
		TEXT("")
	);
	GameplayTags.Notify_Attack_Spawn = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Notify.Attack.Spawn"),
		TEXT("")
	);
	GameplayTags.Notify_Attack_Commit = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Notify.Attack.Commit"),
		TEXT("")
	);
	GameplayTags.Notify_Attack_End = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Notify.Attack.End"),
		TEXT("")
	);
	GameplayTags.Notify_Attack_HitboxOn = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Notify.Attack.HitboxOn"),
		TEXT("")
	);
	GameplayTags.Notify_Attack_HitboxOff = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Notify.Attack.HitboxOff"),
		TEXT("")
	);


	// 몬스터 공격
	GameplayTags.Monster_Attack_Melee = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Monster.Attack.Melee"),
		TEXT("")
	);
	GameplayTags.Monster_Attack_Ranged = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Monster.Attack.Ranged"),
		TEXT("")
	);
	GameplayTags.Monster_Attack_Magic = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Monster.Attack.Magic"),
		TEXT("")
	);
	GameplayTags.Monster_Attack_Special = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Monster.Attack.Special"),
		TEXT("")
	);


	GameplayTags.Boss_Attack_Melee = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Boss.Attack.Melee"),
		TEXT("")
	);
	GameplayTags.Boss_Attack_Ranged = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Boss.Attack.Ranged"),
		TEXT("")
	);
	GameplayTags.Boss_Attack_Magic = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Boss.Attack.Magic"),
		TEXT("")
	);
	GameplayTags.Boss_Attack_Special = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Boss.Attack.Special"),
		TEXT("")
	);
	GameplayTags.Boss_Attack_Special_JumpSlam = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Boss.Attack.Special.JumpSlam"),
		TEXT("")
	);
	GameplayTags.Boss_Attack_Special_Rush = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Boss.Attack.Special.Rush"),
		TEXT("")
	);

	// Trigger
	GameplayTags.Trigger_Hit = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Trigger.Hit"),
		TEXT("")
	);
	GameplayTags.Trigger_OnAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Trigger.OnAttack"),
		TEXT("")
	);
	GameplayTags.Trigger_Projectile_Spawn = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Trigger.Projectile.Spawn"),
		TEXT("")
	);
	GameplayTags.Trigger_Projectile_SpawnFailure = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Trigger.Projectile.SpawnFailure"),
		TEXT("")
	);
	GameplayTags.Trigger_Projectile_Expire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Trigger.Projectile.Expire"),
		TEXT("")
	);
	GameplayTags.Trigger_Projectile_Depleted = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Trigger.Projectile.Depleted"),
		TEXT("")
	);

	// 트리거 추가 정보들
	GameplayTags.AttackType_Direct = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("AttackType.Direct"),
		TEXT("")
	);
	GameplayTags.AttackType_Radial = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("AttackType.Radial"),
		TEXT("")
	);
	GameplayTags.HitType_Environment = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("HitType.Environment"),
		TEXT("")
	);
	

	// Montage
	GameplayTags.Effects_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Effects.HitReact"),
		TEXT("")
	);

	// Combat Socket
	GameplayTags.CombatSocket_Weapon = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("CombatSocket.Weapon"),
		TEXT("")
	);
	GameplayTags.CombatSocket_LHand = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("CombatSocket.LHand"),
		TEXT("")
	);
	GameplayTags.CombatSocket_RHand = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("CombatSocket.RHand"),
		TEXT("")
	);
	GameplayTags.CombatSocket_Head = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("CombatSocket.Head"),
		TEXT("")
	);
	GameplayTags.CombatSocket_Magic = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("CombatSocket.Magic"),
		TEXT("")
	);
	
	GameplayTags.AllElements.Add(GameplayTags.Magic_Element_Fire);
	GameplayTags.AllElements.Add(GameplayTags.Magic_Element_Ice);
	GameplayTags.AllElements.Add(GameplayTags.Magic_Element_Earth);
	GameplayTags.AllElements.Add(GameplayTags.Magic_Element_Wind);
	GameplayTags.AllElements.Add(GameplayTags.Magic_Element_Lightning);

	GameplayTags.ElementsToAffinity.Add(GameplayTags.Magic_Element_Fire, GameplayTags.Attributes_Affinity_FireAffinity);
	GameplayTags.ElementsToAffinity.Add(GameplayTags.Magic_Element_Ice, GameplayTags.Attributes_Affinity_IceAffinity);
	GameplayTags.ElementsToAffinity.Add(GameplayTags.Magic_Element_Earth, GameplayTags.Attributes_Affinity_FireAffinity);
	GameplayTags.ElementsToAffinity.Add(GameplayTags.Magic_Element_Wind, GameplayTags.Attributes_Affinity_FireAffinity);
	GameplayTags.ElementsToAffinity.Add(GameplayTags.Magic_Element_Lightning, GameplayTags.Attributes_Affinity_FireAffinity);

	GameplayTags.ElementsToResistances.Add(GameplayTags.Magic_Element_Fire, GameplayTags.Attributes_Resistance_FireResistance);
	GameplayTags.ElementsToResistances.Add(GameplayTags.Magic_Element_Ice, GameplayTags.Attributes_Resistance_IceResistance);
	GameplayTags.ElementsToResistances.Add(GameplayTags.Magic_Element_Earth, GameplayTags.Attributes_Resistance_EarthResistance);
	GameplayTags.ElementsToResistances.Add(GameplayTags.Magic_Element_Wind, GameplayTags.Attributes_Resistance_WindResistance);
	GameplayTags.ElementsToResistances.Add(GameplayTags.Magic_Element_Lightning, GameplayTags.Attributes_Resistance_LightningResistance);

	GameplayTags.ElementsToAura.Add(GameplayTags.Magic_Element_Fire, GameplayTags.Aura_Fire);
	GameplayTags.ElementsToAura.Add(GameplayTags.Magic_Element_Ice, GameplayTags.Aura_Ice);
	GameplayTags.ElementsToAura.Add(GameplayTags.Magic_Element_Earth, GameplayTags.Aura_Earth);
	GameplayTags.ElementsToAura.Add(GameplayTags.Magic_Element_Wind, GameplayTags.Aura_Wind);
	GameplayTags.ElementsToAura.Add(GameplayTags.Magic_Element_Lightning, GameplayTags.Aura_Lightning);

	GameplayTags.ElementsToGameplayCues.Add(GameplayTags.Magic_Element_Fire, GameplayTags.GameplayCue_Fire);
	GameplayTags.ElementsToGameplayCues.Add(GameplayTags.Magic_Element_Ice, GameplayTags.GameplayCue_Ice);
	GameplayTags.ElementsToGameplayCues.Add(GameplayTags.Magic_Element_Earth, GameplayTags.GameplayCue_Earth);
	GameplayTags.ElementsToGameplayCues.Add(GameplayTags.Magic_Element_Wind, GameplayTags.GameplayCue_Wind);
	GameplayTags.ElementsToGameplayCues.Add(GameplayTags.Magic_Element_Lightning, GameplayTags.GameplayCue_Lightning);

	// Immunity
	GameplayTags.Immunity_Damage = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Immunity.Damage"), FString(""));
	GameplayTags.Immunity_Knockback = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Immunity.Knockback"), FString(""));
	GameplayTags.Immunity_Debuff = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Immunity.Debuff"), FString(""));
	GameplayTags.Immunity_Debuff_Effect_Slow = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Immunity.Debuff.Effect.Slow"), FString(""));
	GameplayTags.Immunity_Debuff_Effect_PeriodicDamage = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Immunity.Debuff.Effect.PeriodicDamage"), FString(""));
	GameplayTags.Immunity_Stun = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Immunity.Stun"), FString(""));

	// Action
	GameplayTags.Action_Damage = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Action.Damage"), FString(""));
	GameplayTags.Action_RadialDamage = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Action.RadialDamage"), FString(""));
	GameplayTags.Action_Knockback = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Action.Knockback"), FString(""));
	GameplayTags.Action_Debuff = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Action.Debuff"), FString(""));

	// Debuff Effect
	GameplayTags.Debuff_Effect_Slow = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Debuff.Effect.Slow"), FString(""));
	GameplayTags.Debuff_Effect_PeriodicDamage = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Debuff.Effect.PeriodicDamage"), FString(""));
}
