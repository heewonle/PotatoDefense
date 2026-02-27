#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "TimerManager.h"

#include "PotatoWaveMetaRow.h"
#include "PotatoWaveSpawnRow.h"
#include "../Core/PotatoEnums.h"

#include "PotatoMonsterSpawner.generated.h"

class APotatoMonster;

UENUM()
enum class EPotatoWaveEndReason : uint8
{
	Cleared,
	GameOver,
	TimeExpired,
	ForcedStop,
};

USTRUCT()
struct FPendingSpawn
{
	GENERATED_BODY()

	FName WaveId = NAME_None;
	EMonsterRank Rank = EMonsterRank::Normal;
	EMonsterType Type = EMonsterType::None;
	int32 Remaining = 0;

	float EntryDelay = 0.f;
	FName SpawnGroup = NAME_None;
	bool bEntryDelayConsumed = false;
};

UCLASS()
class POTATOPROJECT_API APotatoMonsterSpawner : public AActor
{
	GENERATED_BODY()

public:
	APotatoMonsterSpawner();

	// =========================
	// Public Wave API
	// =========================
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StartWave(FName WaveId);

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StopWave();

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void NotifyGameOver();

	UFUNCTION(BlueprintCallable, Category = "Wave")
	void NotifyTimeExpired();

	// =========================
	// Split child spawn (Spawner 파이프라인)
	// =========================
	UFUNCTION(BlueprintCallable, Category = "Wave|Split")
	APotatoMonster* SpawnSplitChildFromParent(
		APotatoMonster* Parent,
		int32 ChildDepth,
		float ChildMaxHpRatio,
		float SpawnJitterRadius,
		float SpawnZOffset
	);

protected:
	// =========================
	// Wave Tables
	// =========================
	UPROPERTY(EditAnywhere, Category = "Wave|Data")
	TObjectPtr<UDataTable> WaveMetaTable = nullptr;

	UPROPERTY(EditAnywhere, Category = "Wave|Data")
	TObjectPtr<UDataTable> WaveSpawnTable = nullptr;

	// =========================
	// Monster preset tables (Spawner -> Monster 주입용)
	// =========================
	UPROPERTY(EditAnywhere, Category = "Monster|Data")
	TObjectPtr<UDataTable> TypePresetTable = nullptr;

	UPROPERTY(EditAnywhere, Category = "Monster|Data")
	TObjectPtr<UDataTable> RankPresetTable = nullptr;

	UPROPERTY(EditAnywhere, Category = "Monster|Data")
	TObjectPtr<UDataTable> SpecialSkillPresetTable = nullptr;

	// =========================
	// Warehouse (직접 지정)
	// =========================
	UPROPERTY(EditAnywhere, Category = "Warehouse")
	TObjectPtr<AActor> DefaultWarehouseActor = nullptr;

	// =========================
	// Spawn class mapping
	// =========================
	UPROPERTY(EditAnywhere, Category = "Monster|Classes")
	TMap<EMonsterType, TSubclassOf<APotatoMonster>> MonsterClassByType;

	// =========================
	// Spawn settings
	// =========================
	UPROPERTY(EditAnywhere, Category = "Wave|Settings")
	float DefaultSpawnInterval = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Wave|Settings")
	float DefaultWaveBaseHP = 0.f;

	// Lane
	UPROPERTY(EditInstanceOnly, Category = "Lane")
	TObjectPtr<AActor> LanePathManager = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Lane")
	FName MoveTargetKeyName = TEXT("MoveTarget");

	// =========================
	// Runtime tracked
	// =========================
	UPROPERTY()
	TArray<TWeakObjectPtr<APotatoMonster>> SpawnedMonsters;

	UPROPERTY()
	int32 AliveCount = 0;

	UPROPERTY()
	bool bWaveActive = false;

	UPROPERTY()
	bool bSpawnFinished = false;

	// ✅ Spawner 내부 전용 등록 루트
	void RegisterSpawnedMonster(APotatoMonster* Monster);

	// ✅ AddDynamic 대상: 반드시 UFUNCTION + (AActor*) 시그니처
	UFUNCTION()
	void HandleSpawnedMonsterDestroyed(AActor* DestroyedActor);

	void EndWave(EPotatoWaveEndReason Reason, bool bClearMonsters);

public:
	// =========================
	// Runtime wave info
	// =========================
	FName CurrentWaveId = NAME_None;
	float CurrentSpawnInterval = 0.5f;
	TArray<FPendingSpawn> PendingQueue;
	FTimerHandle SpawnTickHandle;

	void BuildQueueForWave(FName WaveId, const FPotatoWaveMetaRow& Meta);
	void TickSpawn();

	APotatoMonster* SpawnOne(EMonsterType Type, EMonsterRank Rank, FName SpawnGroup);
	FVector GetSpawnLocationByGroup(FName SpawnGroup) const;

private:
	UPROPERTY(Transient)
	TMap<int32, int32> NextSubWaveIndexByStage;

	UPROPERTY(Transient)
	int32 ActiveStage = INDEX_NONE;

	UPROPERTY(Transient)
	int32 ActiveSubWave = INDEX_NONE;

	UPROPERTY(Transient)
	bool bStageAutoProgress = false;

	bool HasMetaRow(FName WaveId) const;
	bool ResolveFirstWaveForStage(int32 Stage, FName& OutWaveId, int32& OutSub);
	bool ResolveNextWaveForActiveStage(FName& OutWaveId, int32& OutSub);
};