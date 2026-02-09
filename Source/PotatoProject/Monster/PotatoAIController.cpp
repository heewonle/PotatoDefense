#include "PotatoAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"

#include "PotatoMonster.h"

const FName APotatoAIController::Key_WarehouseActor(TEXT("WarehouseActor"));
const FName APotatoAIController::Key_CurrentTarget(TEXT("CurrentTarget"));
const FName APotatoAIController::Key_bIsDead(TEXT("bIsDead"));
const FName APotatoAIController::Key_SpecialLogic(TEXT("SpecialLogic"));

APotatoAIController::APotatoAIController()
{
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
	BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
}

void APotatoAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	APotatoMonster* Monster = Cast<APotatoMonster>(InPawn);
	if (!Monster) return;

	// Possess 타이밍이 BeginPlay보다 빠를 수 있어서(특히 스폰 시)
	// 여기서 한 번 더 프리셋 적용해도 안전하게 동작하도록 설계
	Monster->ApplyPresets();

	UBehaviorTree* BT = Monster->GetBehaviorTreeToRun(); // Resolved BT 우선
	if (!BT || !BT->BlackboardAsset) return;

	BlackboardComp->InitializeBlackboard(*BT->BlackboardAsset);

	// WarehouseActor 주입 (스포너에서 Monster->WarehouseActor 넣어두면 여기서 BB로 전달됨)
	if (Monster->WarehouseActor)
	{
		BlackboardComp->SetValueAsObject(Key_WarehouseActor, Monster->WarehouseActor);
	}
	else
	{
		// 없으면 BB는 비워두고, BTS_UpdateMonsterState에서 나중에 채워도 됨
		BlackboardComp->ClearValue(Key_WarehouseActor);
	}

	// 상태값 초기 동기화
	BlackboardComp->SetValueAsBool(Key_bIsDead, Monster->bIsDead);

	// SpecialLogic: enum을 BB에는 Int로 저장
	// (enum 항목명이 None -> NoSpecial로 바뀌었든 상관 없음. 값은 그대로 int로 들어감)
	BlackboardComp->SetValueAsInt(Key_SpecialLogic, static_cast<int32>(Monster->AppliedSpecialLogic));

	// CurrentTarget 초기화
	BlackboardComp->ClearValue(Key_CurrentTarget);

	RunBehaviorTree(BT);
}
