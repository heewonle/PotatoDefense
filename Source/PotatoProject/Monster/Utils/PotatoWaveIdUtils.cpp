// ===============================
// Utils/PotatoWaveIdUtils.cpp
// ===============================
#include "PotatoWaveIdUtils.h"

bool IsRoundOnlyName(FName WaveId, int32& OutRound)
{
	OutRound = INDEX_NONE;

	const FString S = WaveId.ToString(); // "1" 같은 입력
	if (!S.IsNumeric()) return false;

	OutRound = FCString::Atoi(*S);
	return (OutRound > 0);
}

FName MakeRoundWaveId(int32 Round, int32 Sub)
{
	// 프로젝트에서 쓰는 포맷이 다르면 여기만 바꾸면 됨
	// 현재 스포너 코드가 1-1, 1-2, 1-3 형태를 기대하므로 그대로.
	return FName(*FString::Printf(TEXT("%d-%d"), Round, Sub));
}

bool ParseRoundWaveId(FName WaveId, int32& OutRound, int32& OutSub)
{
	OutRound = INDEX_NONE;
	OutSub = INDEX_NONE;

	const FString S = WaveId.ToString(); // "1-2"
	FString L, R;

	if (!S.Split(TEXT("-"), &L, &R)) return false;
	if (!L.IsNumeric() || !R.IsNumeric()) return false;

	OutRound = FCString::Atoi(*L);
	OutSub = FCString::Atoi(*R);

	return (OutRound > 0 && OutSub > 0);
}