#include "PotatoHardenShellComponent.h"

#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Engine/World.h"
#include "TimerManager.h"

#include "UObject/Class.h"
#include "UObject/UnrealType.h"

// FinalStats
#include "PotatoMonsterFinalStats.h"

UPotatoHardenShellComponent::UPotatoHardenShellComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPotatoHardenShellComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedOwner = GetOwner();
	if (CachedOwner)
	{
		CachedMesh = CachedOwner->FindComponentByClass<USkeletalMeshComponent>();
	}

	// BeginPlay 시점에 HP/MaxHP가 아직 preset 적용 전일 수 있음.
	// 그래서 여기서는 "가능하면"만 초기화하고, 확정 초기화는 InitFromFinalStats에서 한 번 더 한다.
	ResetStepIndexFromCurrentHP();
}

void UPotatoHardenShellComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 타이머 정리 + 원복
	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(HardenRevertTimerHandle);
	}

	RevertHardenMaterial();

	Super::EndPlay(EndPlayReason);
}

void UPotatoHardenShellComponent::InitFromFinalStats(
	const FPotatoMonsterFinalStats& InStats,
	FName InHpPropertyName,
	FName InMaxHpPropertyName
)
{
	// HP reflection 대상 지정
	HpPropertyName = InHpPropertyName;
	MaxHpPropertyName = InMaxHpPropertyName;

	// Data-driven config 주입
	TriggerStepPercent = FMath::Clamp(InStats.HardenTriggerStepPercent, 0.01f, 1.0f);
	DamageMultiplierWhenHardened = FMath::Max(0.f, InStats.HardenDamageMultiplier);
	HardenDurationSeconds = FMath::Max(0.01f, InStats.HardenDurationSeconds);

	TintStrengthParamName = InStats.HardenTintStrengthParamName;
	TintStrengthValue = InStats.HardenTintStrengthValue;

	// 혹시 이전 하든이 남아있으면 상태 정리(프리셋 재적용/리셋 상황 대비)
	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(HardenRevertTimerHandle);
	}
	RevertHardenMaterial();

	// ✅ preset 적용 후 현재 HP 기준으로 스텝 인덱스 확정 초기화
	ResetStepIndexFromCurrentHP();
}

float UPotatoHardenShellComponent::ModifyIncomingDamage(float InDamage) const
{
	if (InDamage <= 0.f) return 0.f;
	if (!bHardened) return InDamage;

	return InDamage * FMath::Max(0.f, DamageMultiplierWhenHardened);
}

bool UPotatoHardenShellComponent::ReadHp(float& OutHP, float& OutMaxHP) const
{
	OutHP = 0.f;
	OutMaxHP = 0.f;

	if (!CachedOwner) return false;

	const UClass* C = CachedOwner->GetClass();
	if (!C) return false;

	FProperty* HpProp = C->FindPropertyByName(HpPropertyName);
	FProperty* MaxProp = C->FindPropertyByName(MaxHpPropertyName);
	if (!HpProp || !MaxProp) return false;

	FFloatProperty* HpFloat = CastField<FFloatProperty>(HpProp);
	FFloatProperty* MaxFloat = CastField<FFloatProperty>(MaxProp);
	if (!HpFloat || !MaxFloat) return false;

	OutHP = HpFloat->GetPropertyValue_InContainer(CachedOwner);
	OutMaxHP = MaxFloat->GetPropertyValue_InContainer(CachedOwner);

	return true;
}

void UPotatoHardenShellComponent::ResetStepIndexFromCurrentHP()
{
	float HP = 0.f, MaxHP = 0.f;
	if (!ReadHp(HP, MaxHP) || MaxHP <= KINDA_SMALL_NUMBER)
	{
		LastStepIndex = INDEX_NONE;
		return;
	}

	const float Step = FMath::Max(0.0001f, TriggerStepPercent);
	const float Pct = FMath::Clamp(HP / MaxHP, 0.f, 1.f);
	LastStepIndex = FMath::FloorToInt(Pct / Step);
}

void UPotatoHardenShellComponent::PostDamageCheck()
{
	// 정책 A: 하든 유지 중 추가 발동 X
	if (bHardened) return;

	float HP = 0.f, MaxHP = 0.f;
	if (!ReadHp(HP, MaxHP)) return;
	if (MaxHP <= KINDA_SMALL_NUMBER) return;

	const float Step = FMath::Max(0.0001f, TriggerStepPercent);
	const float Pct = FMath::Clamp(HP / MaxHP, 0.f, 1.f);

	const int32 CurStepIndex = FMath::FloorToInt(Pct / Step);

	// 최초 세팅이 안됐으면 현재로 맞추고 종료
	if (LastStepIndex == INDEX_NONE)
	{
		LastStepIndex = CurStepIndex;
		return;
	}

	// 스텝 하락 감지 → 1회 발동
	if (CurStepIndex < LastStepIndex)
	{
		LastStepIndex = CurStepIndex;

		bHardened = true;
		ApplyHardenMaterial();

		if (UWorld* W = GetWorld())
		{
			W->GetTimerManager().ClearTimer(HardenRevertTimerHandle);
			W->GetTimerManager().SetTimer(
				HardenRevertTimerHandle,
				this,
				&UPotatoHardenShellComponent::RevertHardenMaterial,
				FMath::Max(0.01f, HardenDurationSeconds),
				false
			);
		}
	}
}

void UPotatoHardenShellComponent::ApplyHardenMaterial()
{
	if (!CachedMesh) return;

	const int32 MatNum = CachedMesh->GetNumMaterials();
	if (MatNum <= 0) return;

	// 원본 저장
	OriginalMaterials.SetNum(MatNum);

	for (int32 i = 0; i < MatNum; i++)
	{
		UMaterialInterface* Orig = CachedMesh->GetMaterial(i);
		OriginalMaterials[i] = Orig;

		if (!Orig) continue;

		UMaterialInstanceDynamic* MID = CachedMesh->CreateAndSetMaterialInstanceDynamic(i);
		if (!MID) continue;

		MID->SetScalarParameterValue(TintStrengthParamName, TintStrengthValue);
	}
}

void UPotatoHardenShellComponent::RevertHardenMaterial()
{
	// 상태부터 해제
	bHardened = false;

	if (!CachedMesh)
	{
		OriginalMaterials.Reset();
		return;
	}

	const int32 MatNum = CachedMesh->GetNumMaterials();
	const int32 SavedNum = OriginalMaterials.Num();
	const int32 N = FMath::Min(MatNum, SavedNum);

	for (int32 i = 0; i < N; i++)
	{
		if (UMaterialInterface* Orig = OriginalMaterials[i])
		{
			CachedMesh->SetMaterial(i, Orig);
		}
	}

	OriginalMaterials.Reset();

	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(HardenRevertTimerHandle);
	}
}