#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "../Core/PotatoEnums.h"
#include "PotatoMonsterTypePreset.generated.h"

USTRUCT(BlueprintType)
struct FPotatoMonsterTypePresetRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BaseHP = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BaseAttackDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BaseAttackRange = 150.0f;

	// 종류별 속도 보정(랭크 속도비율과 곱해짐)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MoveSpeedMultiplier = 1.0f;

	// 종류별 BT 오버라이드(원하면)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UBehaviorTree> OverrideBehaviorTree = nullptr;

	// 확장 슬롯(원거리면 투사체 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsRanged = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> ProjectileClass = nullptr;
};
