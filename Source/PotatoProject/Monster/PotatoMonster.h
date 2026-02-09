#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "BehaviorTree/BehaviorTree.h"

#include "../Core/PotatoEnums.h"
#include "PotatoMonsterRankPreset.h"    // RankPreset row
#include "PotatoMonsterTypePreset.h"    // TypePreset row

#include "PotatoMonster.generated.h"


UCLASS()
class POTATOPROJECT_API APotatoMonster : public ACharacter
{
	GENERATED_BODY()

public:
	APotatoMonster();

protected:
	virtual void BeginPlay() override;

public:
	// =========================
	// Rank: BP 자식 3개에서 고정
	// =========================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Rank")
	EMonsterRank Rank = EMonsterRank::Normal;

	// =========================
	// Type: 너가 원하는 Enum 방식
	// =========================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Type")
	EMonsterType MonsterType = EMonsterType::Zombie;

	// =========================
	// Tables
	// =========================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Data")
	TObjectPtr<UDataTable> RankPresetTable = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Data")
	TObjectPtr<UDataTable> TypePresetTable = nullptr;

	// 웨이브 기본 HP(있으면 TypePreset BaseHP 대신 이걸 베이스로 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Wave")
	float WaveBaseHP = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Balance")
	float PlayerReferenceSpeed = 600.0f;

	// =========================
	// AI
	// =========================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|AI")
	TObjectPtr<UBehaviorTree> DefaultBehaviorTree = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|AI")
	TObjectPtr<UBehaviorTree> ResolvedBehaviorTree = nullptr;

	// =========================
	// Runtime (최종 스탯)
	// =========================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Stats")
	float Health = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Stats")
	float Speed = 300.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Combat")
	float AttackDamage = 10.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Combat")
	float AttackRange = 150.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Combat")
	float StructureDamageMultiplier = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Applied")
	float AppliedHpMultiplier = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Applied")
	float AppliedMoveSpeedRatio = 0.6f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Applied")
	EMonsterSpecialLogic AppliedSpecialLogic = EMonsterSpecialLogic::None;

	// =========================
	// Target / State
	// =========================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Target")
	TObjectPtr<AActor> WarehouseActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Target")
	TObjectPtr<AActor> CurrentTarget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|State")
	bool bIsDead = false;

	// =========================
	// Preset Apply
	// =========================
	UFUNCTION(BlueprintCallable, Category = "Monster|Preset")
	void ApplyPresets(); // TypePreset -> RankPreset

	UFUNCTION(BlueprintCallable, Category = "Monster|AI")
	UBehaviorTree* GetBehaviorTreeToRun() const
	{
		return ResolvedBehaviorTree ? ResolvedBehaviorTree : DefaultBehaviorTree;
	}

	// 기존 API
	UFUNCTION(BlueprintCallable, Category = "Monster|Combat")
	void Attack(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Monster|Combat")
	void ApplyDamage(float Damage);

	UFUNCTION(BlueprintCallable, Category = "Monster|State")
	void Die();

	UFUNCTION(BlueprintCallable, Category = "Monster|Target")
	AActor* FindTarget();

protected:
	void ApplyPresetsFallback();
	static FName GetRankRowName(EMonsterRank InRank);
	static FName GetTypeRowName(EMonsterType InType);
};
