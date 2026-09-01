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
			if (PressedFunc && (Action.AllowedBindings & (uint8)FBindings::BindPress))
			{
				// ETriggerEvent::Started = 입력이 처음 눌린 시점
				BindAction(Action.InputAction, ETriggerEvent::Started, Object, PressedFunc, Action.InputTag);
			}

			if (ReleasedFunc && (Action.AllowedBindings & (uint8)FBindings::BindRelease))
			{
				// ETriggerEvent::Completed = 입력이 해제된 시점
				BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag);
			}

			if (HeldFunc && (Action.AllowedBindings & (uint8)FBindings::BindHeld))
			{
				// ETriggerEvent::Triggered = 입력이 유지되는 동안 매 프레임
				BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, HeldFunc, Action.InputTag);
			}
		}
	}
}
