#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PotatoHardenShellComponent.generated.h"

class USkeletalMeshComponent;
class UMaterialInterface;
struct FPotatoMonsterFinalStats;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class POTATOPROJECT_API UPotatoHardenShellComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPotatoHardenShellComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// =========================
	// External API
	// =========================

	// 데미지 감쇠 적용
	float ModifyIncomingDamage(float InDamage) const;

	// 데미지 적용 후 호출 (HP 스텝 체크)
	void PostDamageCheck();

	// ✅ FinalStats 기반 데이터드리븐 초기화
	void InitFromFinalStats(
		const FPotatoMonsterFinalStats& InStats,
		FName InHpPropertyName,
		FName InMaxHpPropertyName
	);

private:
	// =========================
	// Internal
	// =========================
	bool ReadHp(float& OutHP, float& OutMaxHP) const;

	void ApplyHardenMaterial();
	void RevertHardenMaterial();

	void ResetStepIndexFromCurrentHP();

private:
	// =========================
	// Cached References
	// =========================
	UPROPERTY()
	AActor* CachedOwner = nullptr;

	UPROPERTY()
	USkeletalMeshComponent* CachedMesh = nullptr;

	// =========================
	// Runtime State
	// =========================
	UPROPERTY(VisibleAnywhere, Category="Harden")
	bool bHardened = false;

	int32 LastStepIndex = INDEX_NONE;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInterface>> OriginalMaterials;

	FTimerHandle HardenRevertTimerHandle;

	// =========================
	// Config (Data-driven, private 유지)
	// =========================

	// 0.10 = 10% 단위
	UPROPERTY(EditAnywhere, Category="Harden", meta=(ClampMin="0.01", ClampMax="1.0"))
	float TriggerStepPercent = 0.10f;

	// 0.5 = 50% 데미지
	UPROPERTY(EditAnywhere, Category="Harden", meta=(ClampMin="0.0"))
	float DamageMultiplierWhenHardened = 0.5f;

	// 유지 시간(초)
	UPROPERTY(EditAnywhere, Category="Harden", meta=(ClampMin="0.01"))
	float HardenDurationSeconds = 10.0f;

	// HP reflection 대상
	UPROPERTY(EditAnywhere, Category="Harden|HP")
	FName HpPropertyName = TEXT("HP");

	UPROPERTY(EditAnywhere, Category="Harden|HP")
	FName MaxHpPropertyName = TEXT("MaxHP");

	// Material 파라미터
	UPROPERTY(EditAnywhere, Category="Harden|Material")
	FName TintStrengthParamName = TEXT("TintStrength");

	UPROPERTY(EditAnywhere, Category="Harden|Material")
	float TintStrengthValue = 1.0f;
};