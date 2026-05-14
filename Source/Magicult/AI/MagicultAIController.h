// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MagicultAIController.generated.h"

class UBehaviorTreeComponent;
/**
 * 
 */
UCLASS()
class MAGICULT_API AMagicultAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AMagicultAIController();
protected:

	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;
};
