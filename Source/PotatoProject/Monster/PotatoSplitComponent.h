#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "PotatoMonsterFinalStats.h" // FPotatoSplitSpec
#include "PotatoSplitComponent.generated.h"

class APotatoMonster;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class POTATOPROJECT_API UPotatoSplitComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPotatoSplitComponent();

protected:
	virtual void BeginPlay() override;

public:
	// Apply from FinalStats
	void ApplySpecFromFinalStats(const FPotatoSplitSpec& InSpec, bool bEnable);

	// Called from Monster::TakeDamage after HP applied
	void PostDamageCheck();

	// Depth (Spawner가 child 생성 후 SetSplitDepth로 주입)
	UFUNCTION(BlueprintCallable, Category="Split")
	void SetSplitDepth(int32 InDepth) { SplitDepth = FMath::Max(0, InDepth); }

private:
	// ---- Init ----
	void InitializeFromOwner(APotatoMonster* InOwner);

	// ---- Split conditions ----
	bool CanSplitNow() const;
	bool IsBelowNextThreshold() const;

	// ---- Execute ----
	void DoSplit();

private:
	// Owner cache
	UPROPERTY(Transient)
	TWeakObjectPtr<APotatoMonster> OwnerMonster;

	// Thresholds
	UPROPERTY(EditAnywhere, Category="Split|Spec")
	TArray<float> ThresholdPercents;

	UPROPERTY(Transient)
	int32 NextThresholdIndex = 0;

	// Gates
	UPROPERTY(EditAnywhere, Category="Split|Spec")
	float MinMaxHpToAllowSplit = 0.f;

	UPROPERTY(EditAnywhere, Category="Split|Spec")
	int32 MaxSplitDepth = 0;

	UPROPERTY(Transient)
	int32 SplitDepth = 0;

	// Spawn params
	UPROPERTY(EditAnywhere, Category="Split|Spawn")
	int32 SpawnCount = 1;

	UPROPERTY(EditAnywhere, Category="Split|Spawn", meta=(ClampMin="0.01"))
	float ChildMaxHpRatio = 0.5f;

	UPROPERTY(EditAnywhere, Category="Split|Spawn", meta=(ClampMin="0.0"))
	float SpawnJitterRadius = 60.f;

	UPROPERTY(EditAnywhere, Category="Split|Spawn")
	float SpawnZOffset = 0.f;

	// Scale policy (옵션 A)
	UPROPERTY(EditAnywhere, Category="Split|Scale", meta=(ClampMin="0.01"))
	float OwnerScaleMultiplier = 0.85f;   // 분열 시 부모 스케일 누적 감소

	UPROPERTY(EditAnywhere, Category="Split|Scale")
	bool bScaleChildOnSpawn = true;

	UPROPERTY(EditAnywhere, Category="Split|Scale", meta=(ClampMin="0.01"))
	float ChildScaleMultiplier = 1.0f;    // 자식 = (부모 현재 스케일) * 이 값

	UPROPERTY(EditAnywhere, Category="Split|Scale", meta=(ClampMin="0.001"))
	float MinActorScale = 0.08f;          // 너무 작아지는 것 방지

	// Multi split in same TakeDamage guard
	UPROPERTY(Transient)
	bool bSplitTriggeredThisHit = false;
};