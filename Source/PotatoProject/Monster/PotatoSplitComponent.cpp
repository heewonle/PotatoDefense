// PotatoSplitComponent.cpp (Spawner-linked)
// - Split Child는 반드시 Spawner를 통해 스폰 (AliveCount/Destroy handler/웨이브 클리어 연동)
// - Parent/Child 컨텍스트(테이블/Lane/Warehouse/SpawnerRef)는 Spawner에서 복사 주입
// - TakeDamage 한 번에서 다중 Split 방지(bSplitTriggeredThisHit)
// - Threshold 내림차순 정렬, MaxHP minimum gate, Depth gate 적용
// - ✅ 옵션 A: 분열마다 부모/자식 모두 "같은 규칙"으로 스케일 적용
//   * Parent: Scale *= OwnerScaleMultiplier
//   * Child : Scale  = ParentScaleAfter * ChildScaleMultiplier (ApplyPresetsOnce 이후에 확정)

#include "PotatoSplitComponent.h"

#include "PotatoMonster.h"
#include "PotatoMonsterSpawner.h"

#include "Engine/World.h"

UPotatoSplitComponent::UPotatoSplitComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPotatoSplitComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerMonster = Cast<APotatoMonster>(GetOwner());
	if (OwnerMonster.IsValid())
	{
		InitializeFromOwner(OwnerMonster.Get());
	}
}

void UPotatoSplitComponent::InitializeFromOwner(APotatoMonster* InOwner)
{
	OwnerMonster = InOwner;

	// 임계치 정렬(내림차순) - 안전
	ThresholdPercents.Sort([](float A, float B) { return A > B; });

	NextThresholdIndex = 0;
	bSplitTriggeredThisHit = false;
}

bool UPotatoSplitComponent::CanSplitNow() const
{
	const APotatoMonster* M = OwnerMonster.Get();
	if (!M) return false;
	if (M->IsActorBeingDestroyed()) return false;
	if (M->bIsDead) return false;

	// Depth 제한
	if (SplitDepth >= MaxSplitDepth) return false;

	// MaxHP 너무 작으면 금지
	if (M->MaxHealth < MinMaxHpToAllowSplit) return false;

	// 더 이상 임계치가 없으면 종료
	if (!ThresholdPercents.IsValidIndex(NextThresholdIndex)) return false;

	return true;
}

bool UPotatoSplitComponent::IsBelowNextThreshold() const
{
	const APotatoMonster* M = OwnerMonster.Get();
	if (!M) return false;

	if (!ThresholdPercents.IsValidIndex(NextThresholdIndex)) return false;
	if (M->MaxHealth <= 0.f) return false;

	const float HpPct = M->Health / M->MaxHealth;
	return HpPct <= ThresholdPercents[NextThresholdIndex];
}

void UPotatoSplitComponent::PostDamageCheck()
{
	APotatoMonster* M = OwnerMonster.Get();
	if (!M) return;

	// 한 번의 TakeDamage 흐름에서 분열이 여러 번 실행되는 것 방지
	if (bSplitTriggeredThisHit)
	{
		// 다음 TakeDamage 호출에서 다시 허용
		bSplitTriggeredThisHit = false;
		return;
	}

	if (!CanSplitNow()) return;
	if (!IsBelowNextThreshold()) return;

	bSplitTriggeredThisHit = true;
	DoSplit();
}

static FVector ClampScaleVector(const FVector& InScale, float MinScale)
{
	const float Min = FMath::Max(0.001f, MinScale);
	return FVector(
		FMath::Max(InScale.X, Min),
		FMath::Max(InScale.Y, Min),
		FMath::Max(InScale.Z, Min)
	);
}

void UPotatoSplitComponent::DoSplit()
{
	APotatoMonster* M = OwnerMonster.Get();
	if (!M) return;

	APotatoMonsterSpawner* Spawner = M->SpawnerRef.Get();
	if (!Spawner)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Split] DoSplit FAIL: SpawnerRef is null. Owner=%s"), *GetNameSafe(M));
		return;
	}

	// 다음 임계치로 진행
	NextThresholdIndex++;

	// =========================
	// 1) 부모 스케일 누적 감소 (옵션 A)
	// =========================
	FVector ParentScaleAfter = M->GetActorScale3D();
	{
		const float Mul = FMath::Max(0.01f, OwnerScaleMultiplier);
		ParentScaleAfter = ClampScaleVector(ParentScaleAfter * Mul, MinActorScale);
		M->SetActorScale3D(ParentScaleAfter);
	}

	// =========================
	// 2) 자식 스폰 (Spawner 파이프라인)
	// =========================
	const int32 NextDepth = SplitDepth + 1;

	const float Ratio  = FMath::Max(0.01f, ChildMaxHpRatio);
	const float Jitter = FMath::Max(0.f, SpawnJitterRadius);
	const float Z      = SpawnZOffset;

	const int32 Count = FMath::Max(1, SpawnCount);

	for (int32 i = 0; i < Count; ++i)
	{
		APotatoMonster* Child = Spawner->SpawnSplitChildFromParent(
			M,
			NextDepth,
			Ratio,
			Jitter,
			Z
		);

		if (!IsValid(Child))
		{
			UE_LOG(LogTemp, Warning, TEXT("[Split] Child spawn failed. Owner=%s i=%d"), *GetNameSafe(M), i);
			continue;
		}

		// ✅ 옵션 A: 자식도 같은 규칙으로 스케일 적용
		// - Spawner가 ApplyPresetsOnce까지 끝낸 뒤 돌아오므로 여기서 세팅하면 덮어쓰기 방지에 가장 안전
		if (bScaleChildOnSpawn)
		{
			const float ChildMul = FMath::Max(0.01f, ChildScaleMultiplier);
			const FVector ChildScale = ClampScaleVector(ParentScaleAfter * ChildMul, MinActorScale);
			Child->SetActorScale3D(ChildScale);
		}

		UE_LOG(LogTemp, Warning, TEXT("[Split] Spawned Child=%s Depth=%d Ratio=%.2f ParentScale=(%.2f %.2f %.2f)"),
			*GetNameSafe(Child),
			NextDepth,
			Ratio,
			ParentScaleAfter.X, ParentScaleAfter.Y, ParentScaleAfter.Z);
	}
}

void UPotatoSplitComponent::ApplySpecFromFinalStats(const FPotatoSplitSpec& InSpec, bool bEnable)
{
	if (!bEnable)
	{
		ThresholdPercents.Empty();
		NextThresholdIndex = 0;
		return;
	}

	ThresholdPercents = InSpec.ThresholdPercents;
	ThresholdPercents.Sort([](float A, float B) { return A > B; });

	MinMaxHpToAllowSplit = InSpec.MinMaxHpToAllowSplit;
	MaxSplitDepth        = InSpec.MaxDepth;
	SpawnCount           = InSpec.SpawnCount;
	OwnerScaleMultiplier = InSpec.OwnerScaleMultiplier;
	ChildMaxHpRatio      = InSpec.ChildMaxHpRatio;
	SpawnJitterRadius    = InSpec.SpawnJitterRadius;
	SpawnZOffset         = InSpec.SpawnZOffset;

	// ✅ ChildScaleMultiplier는 Spec에 없으니 기본은 "부모와 동일" (1.0)
	// 필요하면 BP/디폴트에서 조절
	// ChildScaleMultiplier = 1.0f;

	NextThresholdIndex = 0;

	UE_LOG(LogTemp, Warning, TEXT("[Split] AppliedSpec Enable=1 ThNum=%d MinMaxHp=%.1f MaxDepth=%d SpawnCount=%d OwnerMul=%.2f ChildMul=%.2f"),
		ThresholdPercents.Num(),
		MinMaxHpToAllowSplit,
		MaxSplitDepth,
		SpawnCount,
		OwnerScaleMultiplier,
		ChildScaleMultiplier);
}