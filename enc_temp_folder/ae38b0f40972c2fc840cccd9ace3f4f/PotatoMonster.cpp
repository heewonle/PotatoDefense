#include "PotatoMonster.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

FName APotatoMonster::GetRankRowName(EMonsterRank InRank)
{
	switch (InRank)
	{
	case EMonsterRank::Normal: return FName("Normal");
	case EMonsterRank::Elite:  return FName("Elite");
	case EMonsterRank::Boss:   return FName("Boss");
	default:                   return FName("Normal");
	}
}

FName APotatoMonster::GetTypeRowName(EMonsterType InType)
{
	// ✅ DataTable RowName을 Enum 항목 이름과 동일하게 맞추면 가장 안정적
	// 예) EMonsterType::Zombie -> "Zombie"
	const UEnum* Enum = StaticEnum<EMonsterType>();
	if (!Enum) return NAME_None;

	return Enum->GetNameByValue((int64)InType);
}

APotatoMonster::APotatoMonster()
{
	PrimaryActorTick.bCanEverTick = false;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void APotatoMonster::BeginPlay()
{
	Super::BeginPlay();
	ApplyPresets();
}

void APotatoMonster::ApplyPresetsFallback()
{
	AttackDamage = 10.0f;
	AttackRange = 150.0f;

	AppliedHpMultiplier = (Rank == EMonsterRank::Elite) ? 2.5f :
		(Rank == EMonsterRank::Boss) ? 10.0f : 1.0f;

	AppliedMoveSpeedRatio = (Rank == EMonsterRank::Elite) ? 0.8f : 0.6f;

	StructureDamageMultiplier = (Rank == EMonsterRank::Elite) ? 1.5f :
		(Rank == EMonsterRank::Boss) ? 3.0f : 1.0f;

	AppliedSpecialLogic = (Rank == EMonsterRank::Elite) ? EMonsterSpecialLogic::EliteAOE :
		(Rank == EMonsterRank::Boss) ? EMonsterSpecialLogic::BossCustom : EMonsterSpecialLogic::None;

	const float BaseHP = (WaveBaseHP > 0.f) ? WaveBaseHP : 100.0f;
	MaxHealth = BaseHP * AppliedHpMultiplier;
	Health = MaxHealth;

	Speed = PlayerReferenceSpeed * AppliedMoveSpeedRatio;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = Speed;
	}

	ResolvedBehaviorTree = DefaultBehaviorTree;
}

void APotatoMonster::ApplyPresets()
{
	// -------------------------
	// 1) TypePreset 적용 (Enum -> RowName)
	// -------------------------
	const FPotatoMonsterTypePresetRow* TypeRow = nullptr;

	float TypeBaseHP = 100.0f;
	float TypeBaseAttackDamage = 10.0f;
	float TypeBaseAttackRange = 150.0f;
	float TypeMoveSpeedMul = 1.0f;

	if (TypePresetTable)
	{
		const FName TypeRowName = GetTypeRowName(MonsterType);

		if (TypeRowName != NAME_None)
		{
			TypeRow = TypePresetTable->FindRow<FPotatoMonsterTypePresetRow>(
				TypeRowName, TEXT("APotatoMonster::ApplyPresets(TypePreset)")
			);
		}
	}

	if (TypeRow)
	{
		TypeBaseHP = TypeRow->BaseHP;
		TypeBaseAttackDamage = TypeRow->BaseAttackDamage;
		TypeBaseAttackRange = TypeRow->BaseAttackRange;
		TypeMoveSpeedMul = TypeRow->MoveSpeedMultiplier;

		ResolvedBehaviorTree = TypeRow->OverrideBehaviorTree ? TypeRow->OverrideBehaviorTree : DefaultBehaviorTree;
	}
	else
	{
		ResolvedBehaviorTree = DefaultBehaviorTree;
	}

	AttackDamage = TypeBaseAttackDamage;
	AttackRange = TypeBaseAttackRange;

	const float BaseHP = (WaveBaseHP > 0.f) ? WaveBaseHP : TypeBaseHP;

	// -------------------------
	// 2) RankPreset 적용
	// -------------------------
	const FPotatoMonsterRankPresetRow* RankRow = nullptr;

	if (RankPresetTable)
	{
		const FName RankRowName = GetRankRowName(Rank);
		RankRow = RankPresetTable->FindRow<FPotatoMonsterRankPresetRow>(
			RankRowName, TEXT("APotatoMonster::ApplyPresets(RankPreset)")
		);
	}

	if (!RankRow)
	{
		ApplyPresetsFallback();
		return;
	}

	float HpMul = RankRow->HpMultiplierMin;
	if (RankRow->HpMultiplierMax > RankRow->HpMultiplierMin + KINDA_SMALL_NUMBER)
	{
		HpMul = FMath::FRandRange(RankRow->HpMultiplierMin, RankRow->HpMultiplierMax);
	}

	AppliedHpMultiplier = HpMul;
	MaxHealth = BaseHP * AppliedHpMultiplier;
	Health = MaxHealth;

	AppliedMoveSpeedRatio = RankRow->MoveSpeedRatioToPlayer;
	Speed = PlayerReferenceSpeed * AppliedMoveSpeedRatio * TypeMoveSpeedMul;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = Speed;
	}

	StructureDamageMultiplier = RankRow->StructureDamageMultiplier;
	AppliedSpecialLogic = RankRow->SpecialLogic;
}

void APotatoMonster::Attack(AActor* Target)
{
	if (bIsDead || !Target) return;

	const float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
	if (Dist > AttackRange) return;

	float FinalDamage = AttackDamage;

	// 구조물 배율 적용은 이후 태그/캐스트 확정 시 적용
	UGameplayStatics::ApplyDamage(Target, FinalDamage, GetController(), this, nullptr);
}

void APotatoMonster::ApplyDamage(float Damage)
{
	if (bIsDead || Damage <= 0.f) return;

	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	if (Health <= 0.f) Die();
}

void APotatoMonster::Die()
{
	if (bIsDead) return;
	bIsDead = true;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}
}

AActor* APotatoMonster::FindTarget()
{
	if (CurrentTarget) return CurrentTarget;
	return WarehouseActor;
}
