#include "PotatoMonster.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "PotatoPresetApplier.h"
#include "AIController.h"
#include "BrainComponent.h"

#include "Animation/AnimInstance.h"
#include "PotatoCombatComponent.h"

APotatoMonster::APotatoMonster()
{
	PrimaryActorTick.bCanEverTick = false;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	CombatComp = CreateDefaultSubobject<UPotatoCombatComponent>(TEXT("CombatComp"));
}

void APotatoMonster::BeginPlay()
{
	Super::BeginPlay();
	// ApplyPresetsOnce는 AIController::OnPossess에서 호출
}

void APotatoMonster::ApplyPresetsOnce()
{
	if (bPresetsApplied) return;
	bPresetsApplied = true;

	FinalStats = UPotatoPresetApplier::BuildFinalStats(
		this,
		MonsterType,
		Rank,
		WaveBaseHP,
		PlayerReferenceSpeed,
		TypePresetTable,
		RankPresetTable,
		SpecialSkillPresetTable,
		DefaultBehaviorTree
	);

	MaxHealth = FinalStats.MaxHP;
	Health = MaxHealth;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = FinalStats.MoveSpeed;
	}

	ResolvedBehaviorTree = FinalStats.BehaviorTree;

	if (CombatComp)
	{
		CombatComp->InitFromStats(FinalStats);
	}
	UE_LOG(LogTemp, Warning, TEXT("[Preset] Monster=%s AnimSet=%s"),
		*GetName(),
		*GetNameSafe(AnimSet));
}

float APotatoMonster::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	if (bIsDead)
	{
		return 0.f;
	}

	const float Applied = FMath::Max(0.f, DamageAmount);
	if (Applied <= 0.f)
	{
		return 0.f;
	}

	Health = FMath::Clamp(Health - Applied, 0.f, MaxHealth);
	UE_LOG(LogTemp, Warning, TEXT("[TakeDamage] HP=%.1f Applied=%.1f"), Health, Applied);
	if (Health > 0.f)
	{
		//  피격 티 강화: 스팸 방지 + 잠깐 이동 정지
		TryPlayHitReact();
	}
	else
	{
		Die();
	}

	return Applied;
}

// ------------------------------------------------------------
// HitReact: 스팸 방지 + 잠깐 스턴(이동 정지)
// ------------------------------------------------------------
void APotatoMonster::TryPlayHitReact()
{
	const float Now = GetWorld() ? GetWorld()->TimeSeconds : 0.f;

	// 1) 쿨타임
	if (Now - LastHitReactTime < HitReactCooldown)
	{
		return;
	}

	const UPotatoMonsterAnimSet* AS = GetAnimSet();
	if (!AS || !AS->HitReactMontage)
	{
		return;
	}

	USkeletalMeshComponent* Skel = GetMesh();
	UAnimInstance* AnimInst = Skel ? Skel->GetAnimInstance() : nullptr;
	if (!AnimInst)
	{
		return;
	}

	// 2) 이미 HitReact 재생 중이면 무시(끊김 방지)
	if (AnimInst->Montage_IsPlaying(AS->HitReactMontage))
	{
		return;
	}

	LastHitReactTime = Now;

	// 3) 이동 즉시 정지 + DisableMovement (짧게)
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}

	// 4) 몽타주 재생
	AnimInst->Montage_Play(AS->HitReactMontage, 1.0f);

	// 5) 몽타주 길이에 맞춰 복구 타이머(너무 길어지지 않게 클램프)
	const float Len = AnimInst->Montage_Play(AS->HitReactMontage, 1.0f);
	const float StunTime = FMath::Clamp(Len, 0.1f, HitReactMaxStunTime);

	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(HitReactResumeTH);
		GetWorldTimerManager().SetTimer(
			HitReactResumeTH,
			this,
			&APotatoMonster::ResumeMovementAfterHitReact,
			StunTime,
			false
		);
	}
}

void APotatoMonster::ResumeMovementAfterHitReact()
{
	if (bIsDead) return;

	// 걷기 복구
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}
}

// ------------------------------------------------------------
// Die: AI/BT/Move 완전 중단 + DeathMontage 길이만큼 살려두기
// ------------------------------------------------------------
void APotatoMonster::StopAIForDead()
{
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		AICon->StopMovement();

		if (UBrainComponent* Brain = AICon->BrainComponent)
		{
			Brain->StopLogic(TEXT("Dead"));
		}
	}
}

void APotatoMonster::Die()
{
	if (bIsDead) return;
	bIsDead = true;

	// HitReact 복구 타이머 같은 거 남아있으면 정리
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(HitReactResumeTH);
	}

	//  AI/BT 정지
	StopAIForDead();

	//  이동 완전 정지
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}

	SetActorEnableCollision(false);

	float Life = 2.0f;

	//  죽음 몽타주 재생 + 몽타주 길이 기반 LifeSpan
	const UPotatoMonsterAnimSet* AS = GetAnimSet();

	if (AS && AS->DeathMontage)
	{
		if (UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			const float Len = AnimInst->Montage_Play(AS->DeathMontage, 1.0f);
			UE_LOG(LogTemp, Warning, TEXT("[Die] Montage_Play Len=%.3f Montage=%s AnimBP=%s"),
				Len,
				*GetNameSafe(AS->DeathMontage),
				*GetNameSafe(AnimInst->GetClass()));
			if (Len > 0.f)
			{
				Life = FMath::Max(Life, Len + 0.2f);
			}

		}
	}

	SetLifeSpan(Life);
}

void APotatoMonster::AdvanceLaneIndex()
{
	// LanePoints의 마지막을 넘기면 GetCurrentLaneTarget이 Warehouse로 fallback
	LaneIndex = FMath::Clamp(LaneIndex + 1, 0, LanePoints.Num());
}

AActor* APotatoMonster::GetCurrentLaneTarget() const
{
	if (LanePoints.IsValidIndex(LaneIndex))
	{
		return LanePoints[LaneIndex];
	}
	return WarehouseActor;
}

void APotatoMonster::SetAnimSet(UPotatoMonsterAnimSet* InSet)
{
	AnimSet = InSet;

	// AnimInstance에 주입(로코모션/공격에서 참고 가능)
	//if (USkeletalMeshComponent* Skel = GetMesh())
	//{
	//	if (UAnimInstance* AI = Skel->GetAnimInstance())
	//	{
	//		if (UPotatoMonsterAnimInstance* PMI = Cast<UPotatoMonsterAnimInstance>(AI))
	//		{
	//			PMI->SetAnimSet(AnimSet);
	//		}
	//	}
	//}
}