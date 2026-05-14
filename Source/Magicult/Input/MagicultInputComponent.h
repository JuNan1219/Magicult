// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "MagicultInputConfig.h"
#include "MagicultInputComponent.generated.h"

/**
 * 
 */
UCLASS()
class MAGICULT_API UMagicultInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
	
public:
	template<typename UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
	void BindAbilityActions(const UMagicultInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc);

};

template<typename UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void UMagicultInputComponent::BindAbilityActions(const UMagicultInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc)
{
	check(InputConfig);

	using FBindings = EActionInputBindingFlags;

	for (const auto& Action : InputConfig->AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			// 1. Press 이벤트 바인딩
			// Action.AllowedBindings에 BindPress 플래그가 설정되어 있는지 확인합니다.
			if (PressedFunc && (Action.AllowedBindings & (uint8)FBindings::BindPress))
			{
				// ETriggerEvent::Started는 입력이 처음 눌렸을 때 발생합니다.
				BindAction(Action.InputAction, ETriggerEvent::Started, Object, PressedFunc, Action.InputTag);
			}

			// 2. Release 이벤트 바인딩
			// Action.AllowedBindings에 BindRelease 플래그가 설정되어 있는지 확인합니다.
			if (ReleasedFunc && (Action.AllowedBindings & (uint8)FBindings::BindRelease))
			{
				// ETriggerEvent::Completed는 입력이 완료되었거나 해제되었을 때 발생합니다.
				BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag);
			}

			// 3. Held 이벤트 바인딩
			// Action.AllowedBindings에 BindHeld 플래그가 설정되어 있는지 확인합니다.
			if (HeldFunc && (Action.AllowedBindings & (uint8)FBindings::BindHeld))
			{
				// ETriggerEvent::Triggered는 입력이 계속 유지되고 있는 동안 주기적으로 발생합니다.
				BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, HeldFunc, Action.InputTag);
			}
		}
	}
}
