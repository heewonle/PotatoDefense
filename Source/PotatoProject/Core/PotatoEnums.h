#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h"
#include "PotatoEnums.generated.h"


UENUM(BlueprintType)
enum class EResourceType : uint8
{
    Wood        UMETA(DisplayName = "나무"),
    Stone     UMETA(DisplayName = "돌"),
    Crop   UMETA(DisplayName = "농작물"),
    Livestock        UMETA(DisplayName = "가축")
};

UENUM(BlueprintType)
enum class EMonsterType : uint8
{
    Zombie        UMETA(DisplayName = "좀비"),
};