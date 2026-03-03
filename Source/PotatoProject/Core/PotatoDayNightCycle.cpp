#include "PotatoDayNightCycle.h"
#include "PotatoResourceManager.h"

#include "Engine/World.h"
#include "TimerManager.h"

void UPotatoDayNightCycle::StartSystem(float InDayDuration, float InEveningDuration, float InNightDuration, float InDawnDuration)
{
	if (bIsStarted) return;

	bIsStarted = true;

	CachedDayDuration = InDayDuration;
	CachedEveningDuration = InEveningDuration;
	CachedNightDuration = InNightDuration;
	CachedDawnDuration = InDawnDuration;

	TotalElapsedTime = 0.f;

	GetWorld()->GetTimerManager().SetTimer(TickTimerHandle, this, &UPotatoDayNightCycle::OnTimerTick, 1.f, true);

	EnterDay(CachedDayDuration);
}

void UPotatoDayNightCycle::EndSystem()
{
	if (!bIsStarted) return;

	bIsStarted = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PhaseTimerHandle);
		World->GetTimerManager().ClearTimer(TickTimerHandle);
	}
}

void UPotatoDayNightCycle::Deinitialize()
{
	EndSystem();
	Super::Deinitialize();
}

// UI 갱신용
void UPotatoDayNightCycle::OnTimerTick()
{
	RemainingTime = FMath::Max(0.f, RemainingTime - 1.f);
	TotalElapsedTime += 1.f;

	UE_LOG(LogTemp, Log, TEXT("Remaining Time : %.1f"), RemainingTime);
}

void UPotatoDayNightCycle::ClearPhaseTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PhaseTimerHandle);
	}
}

float UPotatoDayNightCycle::GetPhaseStartElapsedTime(EDayPhase Phase) const
{
	// 기존 로직 규칙과 동일하게 맞춤:
	// - Day 시작: 0
	// - Evening 시작: DayDuration
	// - Night 시작: Day + Evening
	// - Dawn 시작: Day + Evening + Night
	switch (Phase)
	{
	case EDayPhase::Day:     return 0.f;
	case EDayPhase::Evening: return CachedDayDuration;
	case EDayPhase::Night:   return CachedDayDuration + CachedEveningDuration;
	case EDayPhase::Dawn:    return CachedDayDuration + CachedEveningDuration + CachedNightDuration;
	default:                 return 0.f;
	}
}

void UPotatoDayNightCycle::EnterDay(float InDayDuration)
{
	CurrentPhase = EDayPhase::Day;
	RemainingTime = InDayDuration;
	TotalElapsedTime = 0.f;   // 새 사이클 시작 시 리셋
	OnDayStarted.Broadcast();

	GetWorld()->GetTimerManager().SetTimer(
		PhaseTimerHandle, [this]() { EnterEvening(CachedEveningDuration); },
		InDayDuration, false);
}

void UPotatoDayNightCycle::EnterEvening(float InEveningDuration)
{
	CurrentPhase = EDayPhase::Evening;
	RemainingTime = InEveningDuration;
	TotalElapsedTime = CachedDayDuration;   // Day가 끝난 시점
	OnEveningStarted.Broadcast();

	GetWorld()->GetTimerManager().SetTimer(
		PhaseTimerHandle, [this]() { EnterNight(CachedNightDuration); },
		InEveningDuration, false);
}

void UPotatoDayNightCycle::EnterNight(float InNightDuration)
{
	CurrentPhase = EDayPhase::Night;
	RemainingTime = InNightDuration;
	TotalElapsedTime = CachedDayDuration + CachedEveningDuration;   // Day+Evening이 끝난 시점
	OnNightStarted.Broadcast();

	GetWorld()->GetTimerManager().SetTimer(
		PhaseTimerHandle, [this]() { EnterDawn(CachedDawnDuration); },
		InNightDuration, false);
}

void UPotatoDayNightCycle::EnterDawn(float InDawnDuration)
{
	CurrentPhase = EDayPhase::Dawn;
	RemainingTime = InDawnDuration;
	TotalElapsedTime = CachedDayDuration + CachedEveningDuration + CachedNightDuration;   // Day+Evening+Night이 끝난 시점
	OnDawnStarted.Broadcast();

	GetWorld()->GetTimerManager().SetTimer(
		PhaseTimerHandle, [this]() { EnterDay(CachedDayDuration); },
		InDawnDuration, false);
}

// ===============================
// ✅ NEW: ForceToDawn / SkipToPhase
// ===============================

void UPotatoDayNightCycle::ForceToDawn(bool bBroadcast)
{
	SkipToPhase(EDayPhase::Dawn, bBroadcast);
}

void UPotatoDayNightCycle::SkipToPhase(EDayPhase TargetPhase, bool bBroadcast)
{
	if (!bIsStarted) return;

	// 남아있는 페이즈 전환 타이머 즉시 제거
	ClearPhaseTimer();

	// 스킵 진입 시: RemainingTime / TotalElapsedTime을 기존 규칙대로 맞춰주고,
	// 원한다면 Broadcast도 함.
	switch (TargetPhase)
	{
	case EDayPhase::Day:
		// Day는 새 사이클이니까 기존 EnterDay가 TotalElapsedTime=0으로 리셋함
		if (!bBroadcast)
		{
			CurrentPhase = EDayPhase::Day;
			RemainingTime = CachedDayDuration;
			TotalElapsedTime = 0.f;
			// 타이머만 재설정
			GetWorld()->GetTimerManager().SetTimer(
				PhaseTimerHandle, [this]() { EnterEvening(CachedEveningDuration); },
				CachedDayDuration, false);
		}
		else
		{
			EnterDay(CachedDayDuration);
		}
		break;

	case EDayPhase::Evening:
		if (!bBroadcast)
		{
			CurrentPhase = EDayPhase::Evening;
			RemainingTime = CachedEveningDuration;
			TotalElapsedTime = GetPhaseStartElapsedTime(EDayPhase::Evening);
			GetWorld()->GetTimerManager().SetTimer(
				PhaseTimerHandle, [this]() { EnterNight(CachedNightDuration); },
				CachedEveningDuration, false);
		}
		else
		{
			EnterEvening(CachedEveningDuration);
		}
		break;

	case EDayPhase::Night:
		if (!bBroadcast)
		{
			CurrentPhase = EDayPhase::Night;
			RemainingTime = CachedNightDuration;
			TotalElapsedTime = GetPhaseStartElapsedTime(EDayPhase::Night);
			GetWorld()->GetTimerManager().SetTimer(
				PhaseTimerHandle, [this]() { EnterDawn(CachedDawnDuration); },
				CachedNightDuration, false);
		}
		else
		{
			EnterNight(CachedNightDuration);
		}
		break;

	case EDayPhase::Dawn:
	default:
		// ✅ 핵심: Night 남은 시간 무시하고 Dawn으로 즉시 진입
		if (!bBroadcast)
		{
			CurrentPhase = EDayPhase::Dawn;
			RemainingTime = CachedDawnDuration;
			TotalElapsedTime = GetPhaseStartElapsedTime(EDayPhase::Dawn);
			GetWorld()->GetTimerManager().SetTimer(
				PhaseTimerHandle, [this]() { EnterDay(CachedDayDuration); },
				CachedDawnDuration, false);
		}
		else
		{
			EnterDawn(CachedDawnDuration);
		}
		break;
	}
}