#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "../Core/PotatoEnums.h"
#include "PotatoMonsterRankPreset.generated.h"

USTRUCT(BlueprintType)
struct FPotatoMonsterRankPresetRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HpMultiplierMin = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HpMultiplierMax = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MoveSpeedRatioToPlayer = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float StructureDamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMonsterSpecialLogic SpecialLogic = EMonsterSpecialLogic::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpecialCooldown = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpecialDamageMultiplier = 1.0f;
};
