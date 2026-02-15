// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "PotatoBTTask_PickRandomLocation.generated.h"

/**
 * 
 */
UCLASS()
class POTATOPROJECT_API UPotatoBTTask_PickRandomLocation : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
    UPotatoBTTask_PickRandomLocation();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    // 어디서 돌아다닐 것인가?
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector MovingAreaKey;

    // 계산된 좌표를 어디에 저장할 것인가?
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetLocationKey;

};
