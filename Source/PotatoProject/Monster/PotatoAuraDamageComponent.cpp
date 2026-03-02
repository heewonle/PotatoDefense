// PotatoAuraDamageComponent.cpp (DEBUG BUILDABLE + PointDamage 옵션)
//
// 핵심:
// - 플레이어쪽 수정 없이 데미지가 "확실히" 들어가게 하려면 ApplyDamage보다 ApplyPointDamage가 안전한 케이스가 많음
// - BeginOverlap에서 HitResult를 캐시해서 PointDamage의 HitInfo로 넣어줌
// - Hit이 없으면(혹은 오래되면) 간단히 라인트레이스로 HitResult를 만든 뒤 넣어줌
//
// CVars:
// - potato.AuraDmgDebug 0/1
// - potato.AuraDmgDraw 0/1
// - potato.AuraUsePointDamage 0/1  (기본 1: PointDamage 우선)

#include "PotatoAuraDamageComponent.h"

#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"

#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"
#include "Engine/EngineTypes.h" // FHitResult
#include "CollisionQueryParams.h"

const FName UPotatoAuraDamageComponent::AuraSphereName(TEXT("PotatoAuraSphere"));

// ------------------------------------------------------------
// Debug / Behavior CVars
// ------------------------------------------------------------
static TAutoConsoleVariable<int32> CVarAuraDmgDebug(
	TEXT("potato.AuraDmgDebug"),
	1,
	TEXT("0=off, 1=log aura damage overlaps and apply"),
	ECVF_Default
);

static TAutoConsoleVariable<int32> CVarAuraDmgDraw(
	TEXT("potato.AuraDmgDraw"),
	0,
	TEXT("0=off, 1=draw debug sphere radius"),
	ECVF_Default
);

static TAutoConsoleVariable<int32> CVarAuraUsePointDamage(
	TEXT("potato.AuraUsePointDamage"),
	1,
	TEXT("0=Use ApplyDamage, 1=Prefer ApplyPointDamage (recommended when target ignores AnyDamage)"),
	ECVF_Default
);

static FORCEINLINE bool AuraDbgOn()
{
	return CVarAuraDmgDebug.GetValueOnGameThread() != 0;
}

UPotatoAuraDamageComponent::UPotatoAuraDamageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPotatoAuraDamageComponent::Configure(float InRadius, float InDps, float InTickInterval)
{
	Radius = FMath::Max(0.f, InRadius);
	Dps = FMath::Max(0.f, InDps);
	TickInterval = FMath::Max(0.01f, InTickInterval);

	if (Sphere)
	{
		Sphere->SetSphereRadius(Radius, true);  // 반드시 true
		Sphere->UpdateOverlaps();               //  이미 근처에 있는 대상 즉시 반영
	}

	if (AuraDbgOn())
	{
		UE_LOG(LogTemp, Warning, TEXT("[AuraDmg] Configure Owner=%s Radius=%.1f Dps=%.2f Tick=%.3f Tag=%s UsePoint=%d"),
			*GetNameSafe(GetOwner()),
			Radius, Dps, TickInterval,
			RequiredTargetTag.IsNone() ? TEXT("None") : *RequiredTargetTag.ToString(),
			CVarAuraUsePointDamage.GetValueOnGameThread()
		);
	}
}

void UPotatoAuraDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedOwner = GetOwner();
	if (!CachedOwner) return;

	for (UActorComponent* C : CachedOwner->GetComponents())
	{
		if (USphereComponent* S = Cast<USphereComponent>(C))
		{
			if (S->GetFName() == AuraSphereName)
			{
				Sphere = S;
				break;
			}
		}
	}

	if (!Sphere)
	{
		Sphere = NewObject<USphereComponent>(CachedOwner, USphereComponent::StaticClass(), AuraSphereName);
		if (Sphere)
		{
			Sphere->RegisterComponent();
			if (USceneComponent* Root = CachedOwner->GetRootComponent())
			{
				Sphere->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
			}
		}
	}
	if (!Sphere) return;

	Sphere->SetSphereRadius(Radius);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionObjectType(ECC_WorldDynamic);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Sphere->SetGenerateOverlapEvents(true);

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &UPotatoAuraDamageComponent::HandleBeginOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &UPotatoAuraDamageComponent::HandleEndOverlap);

	if (AuraDbgOn())
	{
		UE_LOG(LogTemp, Warning, TEXT("[AuraDmg] BeginPlay Owner=%s Sphere=%s Radius=%.1f Dps=%.2f Tick=%.3f ObjType=%d"),
			*GetNameSafe(CachedOwner),
			*GetNameSafe(Sphere),
			Radius, Dps, TickInterval,
			(int32)Sphere->GetCollisionObjectType()
		);
	}

	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().SetTimer(AuraTickTH, this, &UPotatoAuraDamageComponent::TickAura, TickInterval, true);
	}
}

void UPotatoAuraDamageComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AuraDbgOn())
	{
		UE_LOG(LogTemp, Warning, TEXT("[AuraDmg] EndPlay Owner=%s Targets=%d"),
			*GetNameSafe(GetOwner()),
			OverlappingTargets.Num()
		);
	}

	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(AuraTickTH);
	}

	OverlappingTargets.Reset();

	// ✅ 추가: 캐시된 히트도 정리(헤더에 아래 Map 추가 필요)
	LastSweepHitByTarget.Reset();

	Super::EndPlay(EndPlayReason);
}

bool UPotatoAuraDamageComponent::IsValidTarget(AActor* A) const
{
	return IsValid(A)
		&& !A->IsActorBeingDestroyed()
		&& (RequiredTargetTag.IsNone() || A->ActorHasTag(RequiredTargetTag));
}

void UPotatoAuraDamageComponent::HandleBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!IsValidTarget(OtherActor))
	{
		if (AuraDbgOn())
		{
			UE_LOG(LogTemp, Warning, TEXT("[AuraDmg] BeginOverlap IGNORE Owner=%s Other=%s TagReq=%s"),
				*GetNameSafe(GetOwner()),
				*GetNameSafe(OtherActor),
				RequiredTargetTag.IsNone() ? TEXT("None") : *RequiredTargetTag.ToString()
			);
		}
		return;
	}

	if (OtherActor == CachedOwner) return;

	const int32 Before = OverlappingTargets.Num();
	OverlappingTargets.Add(OtherActor);
	const int32 After = OverlappingTargets.Num();

	// ✅ PointDamage용 HitResult 캐시
	if (bFromSweep)
	{
		LastSweepHitByTarget.Add(OtherActor, SweepResult);
	}
	else
	{
		// Sweep이 아니면 간단히 "소유자->타겟 방향"으로 더미 히트 만들기
		FHitResult Dummy;
		Dummy.Location     = OtherActor->GetActorLocation();
		Dummy.ImpactPoint  = Dummy.Location;
		Dummy.TraceStart   = CachedOwner ? CachedOwner->GetActorLocation() : Dummy.Location;
		Dummy.TraceEnd     = Dummy.Location;
		LastSweepHitByTarget.Add(OtherActor, Dummy);
	}

	if (AuraDbgOn())
	{
		UE_LOG(LogTemp, Warning, TEXT("[AuraDmg] BeginOverlap ADD Owner=%s Other=%s Comp=%s Count %d->%d bSweep=%d"),
			*GetNameSafe(CachedOwner),
			*GetNameSafe(OtherActor),
			*GetNameSafe(OtherComp),
			Before, After,
			bFromSweep ? 1 : 0
		);
	}
}

void UPotatoAuraDamageComponent::HandleEndOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!OtherActor) return;

	const int32 Before = OverlappingTargets.Num();
	OverlappingTargets.Remove(OtherActor);
	const int32 After = OverlappingTargets.Num();

	LastSweepHitByTarget.Remove(OtherActor);

	if (AuraDbgOn())
	{
		UE_LOG(LogTemp, Warning, TEXT("[AuraDmg] EndOverlap REM Owner=%s Other=%s Comp=%s Count %d->%d"),
			*GetNameSafe(CachedOwner),
			*GetNameSafe(OtherActor),
			*GetNameSafe(OtherComp),
			Before, After
		);
	}
}

void UPotatoAuraDamageComponent::TickAura()
{
	if (!CachedOwner || !GetWorld()) return;

	if (CVarAuraDmgDraw.GetValueOnGameThread() != 0)
	{
		DrawDebugSphere(GetWorld(), CachedOwner->GetActorLocation(), Radius, 20, FColor::Green, false, TickInterval, 0, 1.5f);
	}

	if (Dps <= 0.f)
	{
		if (AuraDbgOn())
		{
			UE_LOG(LogTemp, Warning, TEXT("[AuraDmg] Tick SKIP (Dps<=0) Owner=%s Dps=%.2f"), *GetNameSafe(CachedOwner), Dps);
		}
		return;
	}

	const float DamageThisTick = Dps * TickInterval;
	if (DamageThisTick <= 0.f) return;

	AController* InstigatorController = nullptr;
	if (APawn* P = Cast<APawn>(CachedOwner))
	{
		InstigatorController = P->GetController();
	}

	const bool bUsePoint = (CVarAuraUsePointDamage.GetValueOnGameThread() != 0);

	if (AuraDbgOn())
	{
		UE_LOG(LogTemp, Warning, TEXT("[AuraDmg] Tick Owner=%s Targets=%d Damage=%.2f Instigator=%s Mode=%s"),
			*GetNameSafe(CachedOwner),
			OverlappingTargets.Num(),
			DamageThisTick,
			*GetNameSafe(InstigatorController),
			bUsePoint ? TEXT("PointDamage") : TEXT("AnyDamage")
		);
	}

	TArray<TWeakObjectPtr<AActor>> ToRemove;

	for (const TWeakObjectPtr<AActor>& W : OverlappingTargets)
	{
		AActor* T = W.Get();
		if (!IsValidTarget(T))
		{
			ToRemove.Add(W);
			continue;
		}

		TSubclassOf<UDamageType> DT = DamageTypeClass ? *DamageTypeClass : UDamageType::StaticClass();

		if (bUsePoint)
		{
			// ✅ 캐시 히트 우선, 없으면 라인 트레이스로 갱신
			FHitResult Hit;
			if (const FHitResult* CachedHit = LastSweepHitByTarget.Find(T))
			{
				Hit = *CachedHit;
			}
			else
			{
				Hit.Location = T->GetActorLocation();
				Hit.ImpactPoint = Hit.Location;
				Hit.TraceStart = CachedOwner->GetActorLocation();
				Hit.TraceEnd = Hit.Location;
			}

			// (선택) 라인트레이스로 더 정확한 Hit 확보
			{
				FCollisionQueryParams Params(SCENE_QUERY_STAT(AuraPointDamageTrace), false, CachedOwner);
				Params.AddIgnoredActor(CachedOwner);
				FHitResult TraceHit;
				const FVector Start = CachedOwner->GetActorLocation();
				const FVector End = T->GetActorLocation();
				if (GetWorld()->LineTraceSingleByChannel(TraceHit, Start, End, ECC_Visibility, Params))
				{
					if (TraceHit.GetActor() == T)
					{
						Hit = TraceHit;
					}
				}
			}

			const FVector ShotDir = (T->GetActorLocation() - CachedOwner->GetActorLocation()).GetSafeNormal();

			if (AuraDbgOn())
			{
				UE_LOG(LogTemp, Warning, TEXT("[AuraDmg]  - ApplyPointDamage -> %s Amount=%.2f DT=%s Impact=%s"),
					*GetNameSafe(T),
					DamageThisTick,
					*GetNameSafe(DT),
					*Hit.ImpactPoint.ToString()
				);
			}

			UGameplayStatics::ApplyPointDamage(
				T,
				DamageThisTick,
				ShotDir,
				Hit,
				InstigatorController,
				CachedOwner,
				DT
			);
		}
		else
		{
			if (AuraDbgOn())
			{
				UE_LOG(LogTemp, Warning, TEXT("[AuraDmg]  - ApplyDamage -> %s Amount=%.2f DT=%s"),
					*GetNameSafe(T),
					DamageThisTick,
					*GetNameSafe(DT)
				);
			}

			UGameplayStatics::ApplyDamage(
				T,
				DamageThisTick,
				InstigatorController,
				CachedOwner,
				DT
			);
		}
	}

	for (const auto& R : ToRemove)
	{
		OverlappingTargets.Remove(R);
		if (AActor* Dead = R.Get())
		{
			LastSweepHitByTarget.Remove(Dead);
		}
	}
}