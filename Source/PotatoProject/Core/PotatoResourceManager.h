#pragma once

#include "CoreMinimal.h"
#include "PotatoEnums.h"
#include "Subsystems/WorldSubsystem.h"
#include "PotatoResourceManager.generated.h"

class UPotatoDayNightCycle;

/*
    아래 델리게이트를 바인딩해서 HUD 업데이트 로직 등에 사용하면 됩니다.
    하나의 함수에서 EnumType 별로 구분하여, NewValue 값을 받아 처리할 수 있습니다.
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceChanged, EResourceType, Type, int32, NewValue);

// TickableGameObject 인터페이스 포함하여 매 프레임 자원 누적 처리 가능
UCLASS()
class POTATOPROJECT_API UPotatoResourceManager : public UWorldSubsystem
{
	GENERATED_BODY()

private:
    int32 Wood = 0;
    int32 Stone = 0;
    int32 Crop = 0;
    int32 Livestock = 0;

    bool bIsStarted = false;

    void BroadcastAll();
    void BroadcastOne(EResourceType Type);

    UPROPERTY()
    UPotatoDayNightCycle* DayNightCycle;

    // ---- Production ----
    // 등록된 분당 총 생산속도 (모든 ProductionComponent 합산)
    int32 TotalProductionPerMinuteWood = 0;
    int32 TotalProductionPerMinuteStone = 0;
    int32 TotalProductionPerMinuteCrop = 0;
    int32 TotalProductionPerMinuteLivestock = 0;

    // 누적 소수 잔여분 (자원 타입당 1개)
    float AccumulatedWood = 0.f;
    float AccumulatedStone = 0.f;
    float AccumulatedCrop = 0.f;
    float AccumulatedLivestock = 0.f;

    const float ProductionTickInterval = 1.0f; // 1초 간격

    FTimerHandle ProductionTimerHandle;
    void OnProductionTick();

    void AccumulateAndFlush(int32 TotalPerMinute, float& Accumulated, float DeltaTime, EResourceType Type);

public:
    void RegisterProduction(int32 InWood, int32 InStone, int32 InCrop, int32 InLivestock);
    void UnregisterProduction(int32 InWood, int32 InStone, int32 InCrop, int32 InLivestock);

    UFUNCTION(BlueprintPure, Category = "Production")
    int32 GetTotalProductionPerMinute(EResourceType Type) const;

public:
    // ---- Lifecycle ----
    UFUNCTION(BlueprintCallable, Category = "ResourceSystem")
    void StartSystem(int32 InitWood, int32 InitStone, int32 InitCrop, int32 InitLivestock);
    UFUNCTION(BlueprintCallable, Category = "ResourceSystem")
    void EndSystem();
    UFUNCTION(BlueprintPure, Category = "ResourceSystem")
    bool IsSystemStarted() const { return bIsStarted; }

    // ---- Resource API ----
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void AddResource(EResourceType Type, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    bool RemoveResource(EResourceType Type, int32 Amount);

    UFUNCTION(BlueprintPure, Category = "Resource")
    int32 GetResource(EResourceType Type) const;

    UFUNCTION(BlueprintPure, Category = "Resource")
    bool HasEnoughResource(EResourceType Type, int32 Amount) const;

    // ---- Getters ----
    UFUNCTION(BlueprintPure, Category = "Resource")
    int32 GetWood() const { return Wood; } 
    UFUNCTION(BlueprintPure, Category = "Resource")
    int32 GetStone() const { return Stone; }
    UFUNCTION(BlueprintPure, Category = "Resource")
    int32 GetCrop() const { return Crop; }
    UFUNCTION(BlueprintPure, Category = "Resource")
    int32 GetLivestock() const { return Livestock; }

    UPROPERTY(BlueprintAssignable, Category = "Resource|Event")
    FOnResourceChanged OnResourceChanged;
};
