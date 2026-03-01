#include "Engine/EngineTypes.h"

// ------------------------------------------------------------
// Trace Channels (Project Settings > Collision 과 반드시 일치)
// ------------------------------------------------------------

// 예시: MonsterHit = GameTraceChannel3
// 프로젝트 설정에서 몇 번인지 확인해서 맞춰줘야 함

constexpr ECollisionChannel CH_MonsterHit = ECC_GameTraceChannel3;
constexpr ECollisionChannel CH_WeaponTrace = ECC_GameTraceChannel1;