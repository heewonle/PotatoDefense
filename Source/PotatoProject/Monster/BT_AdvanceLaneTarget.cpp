#include "BT_AdvanceLaneTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "PotatoMonster.h"

UBT_AdvanceLaneTarget::UBT_AdvanceLaneTarget()
{
	NodeName = TEXT("Advance Lane Target");

	MoveTargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBT_AdvanceLaneTarget, MoveTargetKey), AActor::StaticClass());
	WarehouseActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBT_AdvanceLaneTarget, WarehouseActorKey), AActor::StaticClass());
}

EBTNodeResult::Type UBT_AdvanceLaneTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	APotatoMonster* Monster = Cast<APotatoMonster>(AICon->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	if (MoveTargetKey.SelectedKeyName.IsNone())
	{
		return EBTNodeResult::Failed;
	}

	const int32 NumPoints = Monster->LanePoints.Num();
	if (NumPoints <= 0)
	{
		return EBTNodeResult::Failed;
	}

	// (0) 이동 중이면 실행 차단 (조용히 통과)
	if (UPathFollowingComponent* PFC = AICon->GetPathFollowingComponent())
	{
		const EPathFollowingStatus::Type Status = PFC->GetStatus();
		if (Status == EPathFollowingStatus::Moving || Status == EPathFollowingStatus::Paused)
		{
			return EBTNodeResult::Succeeded;
		}
	}

	// ---- Warehouse 확보 ----
	AActor* Warehouse = nullptr;
	{
		static const FName WarehouseKeyName(TEXT("WarehouseActor"));
		Warehouse = Cast<AActor>(BB->GetValueAsObject(WarehouseKeyName));
		if (!IsValid(Warehouse))
		{
			Warehouse = Monster->WarehouseActor.Get();
			if (IsValid(Warehouse))
			{
				BB->SetValueAsObject(WarehouseKeyName, Warehouse);
			}
		}
	}

	// (1) LaneIndex 유효성 보정
	if (!Monster->LanePoints.IsValidIndex(Monster->LaneIndex))
	{
		AActor* LastPoint = Monster->LanePoints.Last().Get();
		if (IsValid(LastPoint))
		{
			BB->SetValueAsObject(MoveTargetKey.SelectedKeyName, LastPoint);
		}
		return EBTNodeResult::Succeeded;
	}

	AActor* ExpectedTarget = Monster->LanePoints[Monster->LaneIndex].Get();
	if (!IsValid(ExpectedTarget))
	{
		return EBTNodeResult::Failed;
	}

	// ---------------------------------------------------------
	//  (핵심) "레인 이동 시퀀스"에서만 LaneIndex를 올려야 한다.
	//    따라서 BB의 MoveTarget이 ExpectedTarget(현재 LanePoint)일 때만 진행.
	// ---------------------------------------------------------
	AActor* CurMoveTargetBB = Cast<AActor>(BB->GetValueAsObject(MoveTargetKey.SelectedKeyName));

	// 레인 끝나서 MoveTarget이 Warehouse로 바뀐 상태면 여기선 아무것도 안 함
	if (IsValid(Warehouse) && CurMoveTargetBB == Warehouse)
	{
		return EBTNodeResult::Succeeded;
	}

	// 전투 접근(장애물/창고 접근) 중이면 MoveTarget이 LanePoint가 아닐 수 있음 -> 스킵
	if (CurMoveTargetBB != ExpectedTarget)
	{
		// 혹시 BB가 꼬여서 ExpectedTarget이 아닌데 레인 모드라면 복구만 해줌(선택)
		// 단, 전투 접근 중엔 덮어쓰면 안되니 "레인 모드"일 때만 복구하도록 하는 걸 추천.
		const bool bIsLaneMode = BB->GetValueAsBool(TEXT("bIsLaneMode"));
		if (bIsLaneMode)
		{
			BB->SetValueAsObject(MoveTargetKey.SelectedKeyName, ExpectedTarget);
		}
		return EBTNodeResult::Succeeded;
	}

	// ---------------------------------------------------------
	//  도착 판정
	// ---------------------------------------------------------
	const float Dist2D = FVector::Dist2D(Monster->GetActorLocation(), ExpectedTarget->GetActorLocation());
	const float ArrivedThreshold = 60.f;

	if (Dist2D > ArrivedThreshold)
	{
		return EBTNodeResult::Succeeded;
	}

	// ---------------------------------------------------------
	//  마지막 포인트면: 이제 목적지는 Warehouse로 스위치
	// ---------------------------------------------------------
	if (Monster->LaneIndex >= NumPoints - 1)
	{
		// 레인 모드 종료(있으면)
		// Monster->bIsLaneMode = false;  // (멤버 있으면 사용)

		BB->SetValueAsBool(TEXT("bIsLaneMode"), false);

		if (IsValid(Warehouse))
		{
			BB->SetValueAsObject(MoveTargetKey.SelectedKeyName, Warehouse);
			UE_LOG(LogTemp, Warning, TEXT("[AdvanceLane] EndLane -> MoveTarget=Warehouse (%s)"), *GetNameSafe(Warehouse));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[AdvanceLane] EndLane but Warehouse invalid"));
		}

		return EBTNodeResult::Succeeded;
	}

	// ---------------------------------------------------------
	//  정상 도착 → LaneIndex 증가
	// ---------------------------------------------------------
	Monster->AdvanceLaneIndex();

	AActor* NextTarget = Monster->GetCurrentLaneTarget();
	if (IsValid(NextTarget))
	{
		BB->SetValueAsObject(MoveTargetKey.SelectedKeyName, NextTarget);
		return EBTNodeResult::Succeeded;
	}

	// 예외: NextTarget 없으면 마지막 유지
	AActor* LastPoint = Monster->LanePoints.Last().Get();
	if (IsValid(LastPoint))
	{
		BB->SetValueAsObject(MoveTargetKey.SelectedKeyName, LastPoint);
	}

	return EBTNodeResult::Succeeded;
}