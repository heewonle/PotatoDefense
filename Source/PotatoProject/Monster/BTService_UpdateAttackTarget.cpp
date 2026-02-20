// BTService_UpdateAttackTarget.cpp (FINAL INTEGRATED)
// - Lane-follow 우선
// - 레인/웨어하우스 목표 방향 corridor 상 "앞을 막는" 파괴가능 구조물만 블로커로 선정
// - 캡슐/레인폭이 다양해도 동작하도록 corridor 폭/판정 동적화
// - 블로커 타겟 선정 시 우회 시작을 끊기 위해(필요 시) 타겟 변경 순간 StopMovement 1회
// - 레인 모드 + 타겟이 Warehouse(기본)면 MoveGoalLocation은 Clear -> 레인 MoveTo가 우선 실행
// - 블로커(또는 레인 모드가 아닌 Warehouse 접근)일 때만 MoveGoalLocation(접근점) 세팅

#include "BTService_UpdateAttackTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Components/CapsuleComponent.h"

#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Engine/OverlapResult.h"

#include "PotatoMonster.h"
#include "Building/PotatoPlaceableStructure.h"
#include "Building/PotatoStructureData.h"

// =========================================================
// Helpers (CPP-local)
// =========================================================

static float DistancePointToSegment2D(const FVector& P3, const FVector& A3, const FVector& B3)
{
	const FVector P(P3.X, P3.Y, 0.f);
	const FVector A(A3.X, A3.Y, 0.f);
	const FVector B(B3.X, B3.Y, 0.f);

	const FVector AB = B - A;
	const float AB2 = FVector::DotProduct(AB, AB);

	if (AB2 <= KINDA_SMALL_NUMBER)
	{
		return FVector::Dist(A, P);
	}

	const float T = FMath::Clamp(FVector::DotProduct(P - A, AB) / AB2, 0.f, 1.f);
	const FVector Closest = A + T * AB;
	return FVector::Dist(Closest, P);
}

static UPrimitiveComponent* FindFirstCollisionPrimitive(AActor* Target)
{
	if (!Target) return nullptr;

	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Target->GetRootComponent()))
	{
		if (RootPrim->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
			return RootPrim;
	}

	TArray<UPrimitiveComponent*> Prims;
	Target->GetComponents<UPrimitiveComponent>(Prims);
	for (UPrimitiveComponent* C : Prims)
	{
		if (C && C->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
			return C;
	}
	return nullptr;
}

static FVector ClosestPointOnAABB2D(const FVector& Point, const FVector& Origin, const FVector& Extent)
{
	FVector Closest;
	Closest.X = FMath::Clamp(Point.X, Origin.X - Extent.X, Origin.X + Extent.X);
	Closest.Y = FMath::Clamp(Point.Y, Origin.Y - Extent.Y, Origin.Y + Extent.Y);
	Closest.Z = Point.Z;
	return Closest;
}

static bool GetClosestPoint2DOnTarget(AActor* Target, const FVector& From, FVector& OutClosest2D)
{
	if (!Target) return false;

	if (UPrimitiveComponent* Prim = FindFirstCollisionPrimitive(Target))
	{
		FVector Closest3D = FVector::ZeroVector;
		const float Dist = Prim->GetClosestPointOnCollision(From, Closest3D);
		if (Dist >= 0.f)
		{
			OutClosest2D = Closest3D;
			OutClosest2D.Z = 0.f;
			return true;
		}
	}

	FVector Origin, Extent;
	Target->GetActorBounds(true, Origin, Extent);

	OutClosest2D = ClosestPointOnAABB2D(From, Origin, Extent);
	OutClosest2D.Z = 0.f;
	return true;
}

static bool ComputeApproachPoint2D(APotatoMonster* M, AActor* Target, float ExtraOffset, FVector& OutPoint)
{
	if (!M || !Target) return false;

	const FVector From = M->GetActorLocation();

	if (UPrimitiveComponent* Prim = FindFirstCollisionPrimitive(Target))
	{
		FVector Closest = FVector::ZeroVector;
		const float Dist = Prim->GetClosestPointOnCollision(From, Closest);
		if (Dist >= 0.f)
		{
			const FVector Dir = (From - Closest).GetSafeNormal2D();
			OutPoint = Closest + Dir * ExtraOffset;
			OutPoint.Z = From.Z;
			return true;
		}
	}

	FVector Origin, Extent;
	Target->GetActorBounds(true, Origin, Extent);

	const FVector Closest = ClosestPointOnAABB2D(From, Origin, Extent);
	const FVector Dir = (From - Closest).GetSafeNormal2D();

	OutPoint = Closest + Dir * ExtraOffset;
	OutPoint.Z = From.Z;
	return true;
}

static bool IsAliveDestructibleStructure(AActor* A)
{
	if (APotatoPlaceableStructure* S = Cast<APotatoPlaceableStructure>(A))
	{
		if (S->StructureData && S->StructureData->bIsDestructible)
		{
			return (S->CurrentHealth > 0.f);
		}
	}
	return false;
}

static float GetMonsterCapsuleRadiusSafe(const APotatoMonster* M, float Fallback = 34.f)
{
	if (!M) return Fallback;
	if (const UCapsuleComponent* Cap = M->FindComponentByClass<UCapsuleComponent>())
	{
		return Cap->GetScaledCapsuleRadius();
	}
	return Fallback;
}

// =========================================================
// Service
// =========================================================

UBTService_UpdateAttackTarget::UBTService_UpdateAttackTarget()
{
	NodeName = TEXT("Update AttackTarget (Orthodox)");
	Interval = 0.15f;

	AttackTargetKey.SelectedKeyName = TEXT("CurrentTarget");
	InAttackRangeKey.SelectedKeyName = TEXT("bInAttackRange");
}

uint16 UBTService_UpdateAttackTarget::GetInstanceMemorySize() const
{
	return sizeof(FUpdateAttackTargetMemory);
}

bool UBTService_UpdateAttackTarget::ComputeInAttackRange(APotatoMonster* M, AActor* Target, float Range) const
{
	if (!M || !Target) return false;

	FVector From = M->GetActorLocation();
	float CapsuleR = 34.f;
	if (UCapsuleComponent* Cap = M->FindComponentByClass<UCapsuleComponent>())
	{
		From = Cap->GetComponentLocation();
		CapsuleR = Cap->GetScaledCapsuleRadius();
	}

	const float EffectiveRange = Range + CapsuleR;
	const float RangeSq = FMath::Square(EffectiveRange);

	if (UPrimitiveComponent* Prim = FindFirstCollisionPrimitive(Target))
	{
		FVector ClosestPoint = FVector::ZeroVector;
		const float Dist3D = Prim->GetClosestPointOnCollision(From, ClosestPoint);
		if (Dist3D >= 0.f)
		{
			return FVector::DistSquared2D(From, ClosestPoint) <= RangeSq;
		}
	}

	FVector Origin, Extent;
	Target->GetActorBounds(true, Origin, Extent);

	const FVector Closest = ClosestPointOnAABB2D(From, Origin, Extent);
	return FVector::DistSquared2D(From, Closest) <= FMath::Square(EffectiveRange + BoundsRangePadding);
}

// ---------------------------------------------------------
//  FINAL: Dynamic corridor blocker picker
//  - corridor width: capsule 기반(레인폭 몰라도 OK)
//  - 전방성(dot) + 선분거리 + 앞쪽거리(Along) 기반으로 "먼저 막는" 블로커 우선
// ---------------------------------------------------------
AActor* UBTService_UpdateAttackTarget::FindBestBlockerOnCorridor(
	APotatoMonster* M,
	float SearchRange,
	const FVector& CorridorGoalLoc) const
{
	if (!M || !M->GetWorld()) return nullptr;

	UWorld* World = M->GetWorld();

	const FVector MyLoc = M->GetActorLocation();

	// 내 캡슐 기반 corridor 폭 (레인 폭/몬스터 종류 달라도 대응)
	const float MyRadius = GetMonsterCapsuleRadiusSafe(M, 34.f);
	const float CorridorWidth = FMath::Max(80.f, MyRadius * 2.2f + 30.f); // 튜닝: 2.2~3.0, +20~60

	const FVector Goal2D(CorridorGoalLoc.X, CorridorGoalLoc.Y, MyLoc.Z);
	FVector ToGoal2D = (Goal2D - MyLoc).GetSafeNormal2D();
	if (ToGoal2D.IsNearlyZero())
	{
		// goal이 거의 같은 위치면 corridor 판단 불가
		return nullptr;
	}

	TArray<FOverlapResult> Overlaps;
	const FCollisionShape Sphere = FCollisionShape::MakeSphere(SearchRange);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(PotatoTargetingCorridor), false);
	Params.AddIgnoredActor(M);

	const FCollisionObjectQueryParams ObjParams = FCollisionObjectQueryParams::AllObjects;

	const bool bHit = World->OverlapMultiByObjectType(
		Overlaps,
		MyLoc,
		FQuat::Identity,
		ObjParams,
		Sphere,
		Params
	);

	if (!bHit) return nullptr;

	APotatoPlaceableStructure* Best = nullptr;
	float BestScore = BIG_NUMBER;

	for (const FOverlapResult& R : Overlaps)
	{
		AActor* A = R.GetActor();
		if (!IsValid(A)) continue;

		APotatoPlaceableStructure* S = Cast<APotatoPlaceableStructure>(A);
		if (!S || !S->StructureData) continue;
		if (!S->StructureData->bIsDestructible) continue;
		if (S->CurrentHealth <= 0.f) continue;

		FVector SPoint2D;
		if (!GetClosestPoint2DOnTarget(S, MyLoc, SPoint2D))
			continue;

		const FVector SLoc2D(SPoint2D.X, SPoint2D.Y, MyLoc.Z);

		// 1) 전방성: 목표 방향 앞에 있는 것만
		const FVector ToStruct2D = (SLoc2D - MyLoc).GetSafeNormal2D();
		const float ForwardDot = FVector::DotProduct(ToGoal2D, ToStruct2D);
		if (ForwardDot < 0.0f) // 더 빡세게 하려면 0.3~0.5
			continue;

		// 2) corridor(내->goal 선분)에서 벗어난 정도
		const float SegDist = DistancePointToSegment2D(SLoc2D, MyLoc, Goal2D);
		if (SegDist > CorridorWidth)
			continue;

		// 3) goal 방향으로 얼마나 앞에 있는가(작을수록 먼저 막는다)
		const float Along = FVector::DotProduct((SLoc2D - MyLoc), ToGoal2D);
		if (Along < 0.f)
			continue;

		// 4) 점수: 중심에 가까운 놈 + 더 앞쪽에서 막는 놈
		//    SegDist를 강하게 가중해서 corridor 중심에서 벗어난 애 배제
		const float Score = SegDist * 10000.f + Along;

		if (Score < BestScore)
		{
			BestScore = Score;
			Best = S;
		}
	}

	return Best;
}

bool UBTService_UpdateAttackTarget::ShouldKeepCurrentStructureTarget(
	APotatoMonster* M,
	AActor* CurrentTarget,
	const FVector& CorridorGoalLoc) const
{
	if (!M || !CurrentTarget) return false;

	APotatoPlaceableStructure* S = Cast<APotatoPlaceableStructure>(CurrentTarget);
	if (!S || !S->StructureData) return false;
	if (!S->StructureData->bIsDestructible) return false;
	if (S->CurrentHealth <= 0.f) return false;

	// KeepWidth도 동적으로: 내 캡슐 기반 + 여유
	const float MyRadius = GetMonsterCapsuleRadiusSafe(M, 34.f);
	const float CorridorWidth = FMath::Max(80.f, MyRadius * 2.2f + 30.f);
	const float KeepWidth = CorridorWidth + FMath::Max(0.f, KeepTargetExtraWidth);

	FVector SPoint2D;
	GetClosestPoint2DOnTarget(S, M->GetActorLocation(), SPoint2D);

	const FVector MyLoc = M->GetActorLocation();
	const FVector Goal2D(CorridorGoalLoc.X, CorridorGoalLoc.Y, MyLoc.Z);
	const FVector SLoc2D(SPoint2D.X, SPoint2D.Y, MyLoc.Z);

	const float SegDist = DistancePointToSegment2D(SLoc2D, MyLoc, Goal2D);
	return (SegDist <= KeepWidth);
}

void UBTService_UpdateAttackTarget::UpdateBlockedState(
	FUpdateAttackTargetMemory& Mem,
	APotatoMonster* M,
	const FVector& CorridorGoalLoc,
	float DeltaSeconds) const
{
	if (!M) return;

	const float Dist = FVector::Dist2D(M->GetActorLocation(), CorridorGoalLoc);

	if (Mem.LastDistToMoveTarget2D < 0.f)
	{
		Mem.LastDistToMoveTarget2D = Dist;
		Mem.NoProgressTime = 0.f;
		Mem.bBlocked = false;
		return;
	}

	const float Delta = Mem.LastDistToMoveTarget2D - Dist; // 전진하면 +
	if (Delta < NoProgressMinDelta)
	{
		Mem.NoProgressTime += DeltaSeconds;
	}
	else
	{
		Mem.NoProgressTime = 0.f;
	}

	Mem.bBlocked = (Mem.NoProgressTime >= NoProgressHoldTime);
	Mem.LastDistToMoveTarget2D = Dist;
}

// ---------------------------------------------------------
//  FINAL: TickNode Integrated
// ---------------------------------------------------------
void UBTService_UpdateAttackTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!BB || !AIC) return;

	APotatoMonster* M = Cast<APotatoMonster>(AIC->GetPawn());
	if (!M || M->bIsDead) return;

	FUpdateAttackTargetMemory* Mem = reinterpret_cast<FUpdateAttackTargetMemory*>(NodeMemory);

	// =========================================================
	// 1) Warehouse 확보
	// =========================================================
	AActor* Warehouse = Cast<AActor>(BB->GetValueAsObject(WarehouseKeyName));
	if (!IsValid(Warehouse))
	{
		Warehouse = M->WarehouseActor.Get();
		if (IsValid(Warehouse))
		{
			BB->SetValueAsObject(WarehouseKeyName, Warehouse);
		}
	}

	// =========================================================
	// 2) Corridor Goal 결정 (레인 모드면 MoveTarget, 아니면 Warehouse)
	// =========================================================
	const bool bIsLaneMode = BB->GetValueAsBool(TEXT("bIsLaneMode"));

	AActor* LaneMoveTarget = nullptr;
	if (bIsLaneMode)
	{
		LaneMoveTarget = Cast<AActor>(BB->GetValueAsObject(TEXT("MoveTarget"))); // 레인 포인트 키
	}

	const FVector MyLoc = M->GetActorLocation();
	const FVector CorridorGoalLoc =
		IsValid(LaneMoveTarget) ? LaneMoveTarget->GetActorLocation() :
		(IsValid(Warehouse) ? Warehouse->GetActorLocation() : MyLoc);

	// =========================================================
	// 3) 막힘 감지(옵션) - CorridorGoalLoc 기준
	// =========================================================
	if (Mem)
	{
		UpdateBlockedState(*Mem, M, CorridorGoalLoc, DeltaSeconds);
	}

	const float Range = M->FinalStats.AttackRange;

	// =========================================================
	// 4) 공격 중이면 타겟 재선정 잠금
	// =========================================================
	const bool bIsAttacking = BB->GetValueAsBool(TEXT("bIsAttacking"));

	AActor* Current = Cast<AActor>(BB->GetValueAsObject(AttackTargetKey.SelectedKeyName));
	AActor* Target = nullptr;

	if (bIsAttacking && IsValid(Current))
	{
		if (Current == Warehouse || IsAliveDestructibleStructure(Current))
		{
			Target = Current;
		}
	}

	// =========================================================
	// 5) 타겟 선정: 기본 Warehouse, corridor 상 blocker 있으면 blocker 우선
	// =========================================================
	if (!IsValid(Target))
	{
		Target = Warehouse;

		// 현재 구조물 타겟 유지(지금도 corridor 위에 있으면 유지)
		if (IsValid(Current) && Current != Warehouse && ShouldKeepCurrentStructureTarget(M, Current, CorridorGoalLoc))
		{
			Target = Current;
		}
		else
		{
			// 레인 폭/캡슐이 다양하니 레인 모드에서는 최소 탐색 반경을 크게 잡아 "우회 시작 전" 발견
			float SearchRange = FMath::Max(Range + CorridorSearchExtraRange, MinCorridorSearchRange);

			if (bIsLaneMode)
			{
				SearchRange = FMath::Max(SearchRange, 800.f); //  테스트 기본값(레벨 스케일에 맞춰 600~1200 조정)
			}

			if (Mem && Mem->bBlocked)
			{
				SearchRange += BlockedSearchExtra;
			}

			if (AActor* Blocker = FindBestBlockerOnCorridor(M, SearchRange, CorridorGoalLoc))
			{
				Target = Blocker;
			}
		}
	}

	// =========================================================
	// 6) 사거리
	// =========================================================
	const bool bInRange = IsValid(Target) ? ComputeInAttackRange(M, Target, Range) : false;

	BB->SetValueAsObject(AttackTargetKey.SelectedKeyName, Target);
	BB->SetValueAsBool(InAttackRangeKey.SelectedKeyName, bInRange);

	const bool bTargetIsWarehouse = (IsValid(Target) && IsValid(Warehouse) && Target == Warehouse);
	const bool bTargetIsBlocker = (IsValid(Target) && Target != Warehouse && IsAliveDestructibleStructure(Target));

	// =========================================================
	// 7) Target 변경 순간 "우회 시작" 끊기 (선택이지만 추천)
	//    - 너무 자주 StopMovement()하면 떨릴 수 있으니 "타겟 변경"일 때만
	// =========================================================
	if (Mem)
	{
		// FUpdateAttackTargetMemory에 아래가 없으면 헤더에 추가 필요:
		// TWeakObjectPtr<AActor> LastTarget;
		if (Mem->LastTarget.Get() != Target)
		{
			// 블로커를 새로 잡은 순간이면 path를 끊어 우회 시작을 방지
			if (bTargetIsBlocker)
			{
				AIC->StopMovement();
			}
			Mem->LastTarget = Target;
		}
	}

	// =========================================================
	// 8) MoveGoalLocation 운영 규칙 (전투/접근 전용)
	//
	//  - Target 유효 X      : Clear
	//  - 사거리 안(bInRange): StopMovement + 현재 위치로 고정
	//  - 사거리 밖          :
	//      * 레인모드 + 타겟이 Warehouse(기본) => Clear (레인 MoveTarget 우선)
	//      * blocker 타겟 or (레인모드가 아님) => ApproachPoint 세팅
	// =========================================================
	if (!IsValid(Target))
	{
		BB->ClearValue(MoveGoalLocationKeyName);
	}
	else if (bInRange)
	{
		// 사거리 들어온 순간, 우회 경로 타기 전에 이동 끊기
		AIC->StopMovement();
		BB->SetValueAsVector(MoveGoalLocationKeyName, MyLoc);
	}
	else
	{
		// 레인 따라가는 중 + 타겟이 Warehouse(기본)면 MoveGoalLocation은 비워둔다
		// -> BT 레인 이동(MoveTo: MoveTarget)만 타게 됨
		if (bIsLaneMode && bTargetIsWarehouse && !bTargetIsBlocker)
		{
			BB->ClearValue(MoveGoalLocationKeyName);
		}
		else
		{
			const float CapsuleR = GetMonsterCapsuleRadiusSafe(M, 34.f);
			const float Extra = CapsuleR + ApproachExtraOffset;

			FVector Approach;
			if (ComputeApproachPoint2D(M, Target, Extra, Approach))
			{
				BB->SetValueAsVector(MoveGoalLocationKeyName, Approach);
			}
			else
			{
				BB->SetValueAsVector(MoveGoalLocationKeyName, Target->GetActorLocation());
			}
		}
	}

	// (선택) Object Goal은 디버그/표시용
	BB->SetValueAsObject(MoveGoalKeyName, Target);
}