// Fill out your copyright notice in the Description page of Project Settings.


#include "Animal/PotatoBTTask_PickRandomLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/BoxComponent.h"
#include "NavigationSystem.h" 
#include "NavFilters/NavigationQueryFilter.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"

UPotatoBTTask_PickRandomLocation::UPotatoBTTask_PickRandomLocation()
{
    NodeName = TEXT("Pick Random Location");
    
    // Moving Area 필터링 - BoxComponent만 허용
    MovingAreaKey.AddObjectFilter(this, 
        GET_MEMBER_NAME_CHECKED(UPotatoBTTask_PickRandomLocation, MovingAreaKey), UBoxComponent::StaticClass());

    // Target Location - Vector
    TargetLocationKey.AddVectorFilter(this,
        GET_MEMBER_NAME_CHECKED(UPotatoBTTask_PickRandomLocation, TargetLocationKey));
}

EBTNodeResult::Type UPotatoBTTask_PickRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("PickRandomLocation: No BlackboardComponent found."));
        return EBTNodeResult::Failed;
    }

    UObject* Building = BlackboardComp->GetValueAsObject(MovingAreaKey.SelectedKeyName);
    UBoxComponent* TargetArea = Cast<UBoxComponent>(Building);

    if (!TargetArea)
    {
        UE_LOG(LogTemp, Warning, TEXT("PickRandomLocation: Invalid BoxComponent in MovingAreaKey."));
        return EBTNodeResult::Failed;
    }

    // 랜덤 위치 계산
    FVector Origin = TargetArea->GetComponentLocation();
    FVector BoxExtent = TargetArea->GetScaledBoxExtent();
    FVector RandomPoint = UKismetMathLibrary::RandomPointInBoundingBox(Origin, BoxExtent);

    UWorld* World = OwnerComp.GetWorld();
    UNavigationSystemV1* NavSys = World ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;

    if (NavSys)
    {
        FNavLocation Projected;
        const FVector Extent(200.f, 200.f, 200.f);

        // NavData 가져오기 (기본 네비 데이터)
        const ANavigationData* NavData = NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);

        // FilterClass -> 실제 QueryFilter로 변환
        FSharedConstNavQueryFilter QueryFilter;
        if (NavData && NavFilterClass)
        {
            QueryFilter = UNavigationQueryFilter::GetQueryFilter(*NavData, OwnerComp.GetAIOwner(), NavFilterClass);
        }

        if (NavData && NavSys->ProjectPointToNavigation(RandomPoint, Projected, Extent, NavData, QueryFilter))
        {
            BlackboardComp->SetValueAsVector(TargetLocationKey.SelectedKeyName, Projected.Location);
            return EBTNodeResult::Succeeded;
        }
    }

    BlackboardComp->SetValueAsVector(TargetLocationKey.SelectedKeyName, RandomPoint);

    return EBTNodeResult::Succeeded;
}
