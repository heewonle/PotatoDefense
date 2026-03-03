// ===============================
// Utils/PotatoWaveIdUtils.h
// ===============================
#pragma once

#include "CoreMinimal.h"

// 기존에 쓰던 것들
bool IsRoundOnlyName(FName WaveId, int32& OutRound);
FName MakeRoundWaveId(int32 Round, int32 Sub);

// 추가: "1-2" 같은 WaveId 파싱
bool ParseRoundWaveId(FName WaveId, int32& OutRound, int32& OutSub);