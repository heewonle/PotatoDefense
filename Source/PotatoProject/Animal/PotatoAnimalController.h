// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PotatoAnimalController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;

/**
 * 
 */
UCLASS()
class POTATOPROJECT_API APotatoAnimalController : public AAIController
{
	GENERATED_BODY()

public:
    APotatoAnimalController();

protected:
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
	
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

    UPROPERTY()
    TObjectPtr<UBlackboardComponent> BlackboardComp;

    UPROPERTY()
    TObjectPtr<UBehaviorTreeComponent> BehaviorComp;

    static const FName Key_BarnActor;
    static const FName Key_BarnBounds;
};
