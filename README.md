# 🥔 파머스 디펜더 (Farmer's Defender) — 감자마을 지키기!

<div align="center">

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![C++](https://img.shields.io/badge/C++-17-00599C.svg?logo=c%2B%2B)
![Platform](https://img.shields.io/badge/platform-UnrealEngine-blue)
![License](https://img.shields.io/badge/license-MIT-green.svg)

**[🎥 Gameplay Video](https://youtu.be/Zvn8c7br6FM?si=ErGDBqv_skyacZLh)** ·
**[📄 Game Design Document](https://www.notion.so/teamsparta/2e12dc3ef51481f48ea1ea812405075d)** ·
**[💻 GitHub 저장소](https://github.com/NbcampUnreal/7th-Team5-CH3-Project)**  

**Unreal Engine 5.6 · C++ · TPS Tower Defense × Farm Management**
<div align="center">
  
> "농작물로 쏘고, 동물로 키우고, 마을을 지켜라!"

---

## 📖 Overview

**파머스 디펜더**는 UE5 기반의 **로우폴리 TPS 타워 디펜스 × 농장 경영 하이브리드 게임**입니다.

플레이어는 **낮 동안 자원을 수확하고 시설을 건설**하며,  
**밤에는 몬스터 웨이브로부터 식량 저장고를 방어**합니다.

무기 탄약은 농작물로 제작되며,  
**경영 판단이 곧 전투력**이 되는 구조입니다.

| | |
|---|---|
| **Genre** | TPS · Tower Defense · Farm Management |
| **Platform** | Windows |
| **Engine** | Unreal Engine 5.6 |
| **Language** | C++ / Blueprint |
| **Dev Period** | 2026.01 ~ 2026.03 |
| **Team** | 5명 |

---

## 🔄 Core Game Loop

```
🌅 낮 페이즈 (Day)
│  건물 건설 · 바리케이드 설치 · NPC 고용 · 동물 배치 · 탄약 제작
│
├─ 30초 경고 → 최종 준비
│
🌙 밤 페이즈 (Night)
│  몬스터 웨이브 방어 (최대 3 웨이브)
│
├─ 생존 성공 → 보상 획득 · NPC 유지비 차감
│
└─ 다음 Day로 (난이도 자동 증가)
```

### 승리 / 패배 조건

| 조건 | 결과 |
|---|---|
| 10일차 최종 웨이브 생존 | 🏆 **Victory** |
| 플레이어 사망 | 💀 Game Over |
| 식량 저장고 파괴 | 💀 Game Over |

---

## 🎮 Core Features

### 🌗 Day / Night Cycle

게임은 **Day → Evening → Night → Dawn** 4페이즈 사이클로 진행됩니다.

| Phase | Description |
|---|---|
| ☀️ Day | 자원 수확, 건물 건설, NPC 고용, 동물 관리, 탄약 제작 |
| 🌅 Evening | 야간 경고 (30초), 최종 준비 |
| 🌙 Night | 몬스터 웨이브 방어 전투 |
| 🌄 Dawn | 결산, NPC 유지비 차감, 생존 보상 |

---

### ⚔️ Combat System

**단일 무기 플랫폼 (Farm Cannon)** 에 **4종의 농작물 탄약**을 슬롯 교체하여 사용합니다.

| Weapon | Type | Feature | 탄창 | 농작물 비용 |
|---|---|---|---|---|
| 🥔 감자 | Projectile | 포물선 탄도, 기본 데미지 | 30발 | 1:1 |
| 🌽 옥수수 | Projectile | 관통 (최대 2회) | 30발 | 1:1 |
| 🎃 호박 | Grenade | 폭발 범위 데미지 (3m) | 10발 | 1:3 |
| 🥕 당근 | Hitscan | 산탄 8발 확산 (±15°) | 20발 | 1:2 |

- **반동 시스템**: Pitch/Yaw 반동 + 무기 모델 킥백
- **탄착군 시스템**: 이동·점프·연사에 따른 동적 스프레드
- **재장전 시스템**: 탄창 + 예비 탄약 관리
- **탄약 제작**: 낮 페이즈에 농작물 자원 → 탄약 전환

---

### 🏗️ Building & Economy

#### 생산 건물

| 건물 | 건설 비용 | 기본 생산 | NPC 효과 |
|---|---|---|---|
| 🌾 밭 | 나무 8 | 농작물 +3/분 | — |
| 🐄 축사 | 나무 12 | 동물 배치로 생산 | 최대 5마리 |
| 🪓 벌목장 | 나무 5 | 나무 +1/분 | 벌목꾼 배치 시 증가 |
| ⛏️ 광산 | 나무 8, 광석 5 | 광석 +0.5/분 | 광부 배치 시 증가 |

#### 바리케이드

| 바리케이드 | HP | 특수 효과 |
|---|---|---|
| 나무 울타리 | 100 | — |
| 석벽 | 250 | 몬스터 이동속도 -20% |
| 수레 | 150 | 엄폐물 |
| 나무통 | 80 | 파괴 시 나무 1 회수 |
| 가시 울타리 | 80 | 접촉 시 10 데미지 |

- **자원 4종**: 🪵 나무 · ⛏️ 광석 · 🌾 농작물 · 🥛 축산물
- **NPC 고용**: 벌목꾼 · 광부 — 생산량 증가, 매 밤 축산물 유지비
- **동물 관리**: 소 (+농작물) · 돼지 (+축산물) · 닭 (+농작물/축산물)

---

### 👾 Monster System

**11종의 몬스터** × **3단계 등급 (Normal / Elite / Boss)**

#### 일반 몬스터

| Monster | 특징 |
|---|---|
| 🐀 들쥐 | 기본 물량형 |
| 🐍 뱀 | 빠른 이동, 낮은 HP |
| 🐒 거미 | 원거리 투척 (거미줄) |
| 🍄 웃는버섯 | 느리지만 높은 HP (탱커) |

#### 엘리트 몬스터

| Monster | Special Mechanic |
|---|---|
| 💀 스켈레톤 | 광역 검 휘두르기 (AoE Slash) |
| 🌵 선인장 | 접촉 지속 데미지 (Contact DOT) |
| 🍄 화난버섯 | 높은 HP, 강화 탱커 |
| 🦈 못난가오리 | 독침 원거리 투사체 |

#### 보스 몬스터

| Monster | Day | Special Mechanic |
|---|---|---|
| 🟢 슬라임 | Day 3 | 분열 (Split) — HP 50%에서 2마리로 분열 |
| 🐢 가시거북 | Day 7 | 껍질 경화 (HardenShell) + 돌진 (Charge) |
| 📦 용 | Day 10 | 불기둥!! |

**몬스터 특수 시스템:**
- **분열 (Split)**: HP 임계치 도달 시 분열체 스폰 (최대 3회)
- **경화 (HardenShell)**: 피해 50% 감쇠 모드
- **오라 데미지 (AuraDamage)**: 접촉 시 지속 데미지
- **특수 스킬 트리거**: OnCooldown / OnHit / OnDeath / OnNearTarget

---

### 🗺️ Wave System

10일간 생존하는 구조이며, Day별 난이도가 자동 증가합니다.

| Day | 테마 | 웨이브 수 | 스폰 방향 | 특징 |
|---|---|---|---|---|
| 1 | 튜토리얼 | 3 | 1방향 | 기본 학습 |
| 2 | 기초 방어 | 3 | 1방향 | 엘리트 첫 등장 |
| **3** | **보스전** | **1** | **1방향** | **🟢 슬라임** |
| 4 | 회복 | 3 | 2방향 | 방향 압박 시작 |
| 5~6 | 압박 증가 | 3 | 2방향 | 엘리트 비율 증가 |
| **7** | **보스전** | **1** | **1방향** | **🐢 가시거북** |
| 8~9 | 긴장 고조 | 3 | 3방향 | 전방위 압박 |
| **10** | **최종 보스** | **1** | **1방향** | **드래곤** |

---

## 🛠 Tech Stack

### Development Environment

| Category | Details |
|---|---|
| Engine | Unreal Engine 5.6 |
| Language | C++17 |
| IDE | Visual Studio 2022 |
| Platform | Windows |
| VCS | Git / GitHub |

### Architecture & Design

```
Source/PotatoProject/
├── Core/                  # GameMode, GameState, DayNightCycle, ResourceManager
│                          # ProductionComponent, RewardGenerator
├── Player/                # PlayerCharacter, PlayerController, AnimInstance
├── Combat/                # WeaponComponent, Projectile, HitscanTrailActor, WeaponData
├── Monster/               # Monster, AI (BehaviorTree), CombatComp, SpecialSkill
│   ├── Utils/             # AnimUtils, RuntimeUtils, TargetGeometry, WaveIdUtils
│   └── FXUtils/           # VFX/SFX 유틸리티
├── Building/              # BuildSystemComponent, PlaceableStructure, StructureData
│                          # AnimalManagementComp, NPCManagementComp
├── NPC/                   # PotatoNPC
├── UI/                    # HUD, HealthBar, DamageText, Crosshair, HitMarker
│                          # AmmoPopup, AnimalPopup, NPCPopup, PauseMenu
│                          # ResultScreen, GameOverScreen
└── Utils/                 # ActorSafety 등 공통 유틸
```

**핵심 설계 원칙:**

- **Data-Driven Design**: `UPrimaryDataAsset` 기반 WeaponData · MonsterPreset (Type/Rank/SpecialSkill 테이블) · StructureData
- **Component Architecture**: CombatComp · SplitComp · HardenShellComp · AuraDamageComp · SpecialSkillComp
- **Collision Profile 분리**: MonsterRoot (이동/Nav) / MonsterHit (피격 전용, 확대 캡슐) 분리
- **FX Budget System**: 거리 게이트 + 글로벌 예산 기반 VFX/SFX 제어
- **DataTable 기반 웨이브**: WaveMeta · WaveSpawn · TypePreset · RankPreset · SpecialSkillPreset

---

## 🎮 Controls

### 이동

| Input | Action |
|---|---|
| W / A / S / D | 이동 |
| Space | 점프 |
| Shift (Hold) | 달리기 (1.5배속) |
| Mouse | 시점 조작 |

### 전투

| Input | Action |
|---|---|
| LMB (Left Click) | 발사 |
| RMB (Right Click) | 조준 (줌 인, 스프레드 감소) |
| R | 재장전 |
| 1 / 2 / 3 / 4 | 무기 슬롯 전환 |

### 건설 모드

| Input | Action |
|---|---|
| B | 건설 모드 진입/종료 |
| LMB | 배치 (초록색일 때) |
| RMB / ESC | 취소 |
| Mouse Wheel | 90° 회전 |
| Q / E | 건물 종류 순환 |

### 기타

| Input | Action |
|---|---|
| F | 상호작용 (건물 UI, 자원 수집) |
| Tab | 탄약 제작 UI |
| ESC | 일시정지 메뉴 |

---

## 💻 Build & Run

### Requirements

- Windows 10 이상
- Unreal Engine 5.6
- Visual Studio 2022

### Clone & Build

```bash
git clone https://github.com/NbcampUnreal/7th-Team5-CH3-Project.git
```

1. `PotatoProject.uproject` 더블클릭으로 에디터 실행
2. 또는 Visual Studio에서 `Development Editor` 빌드

---

## 🧱 Core Systems

- 낮/밤 사이클 시스템 (Day-Night Cycle)
- 자원 관리 시스템 (Resource Manager)
- 자원 생산 시스템 (Production Component)
- 무기 & 전투 시스템 (Weapon Component)
- 몬스터 웨이브 스폰 시스템 (Monster Spawner)
- 그리드 기반 건물 배치 시스템 (Build System Component)
- NPC 관리 시스템 (NPC Management Component)
- 동물 관리 시스템 (Animal Management Component)
- 데미지 텍스트 풀링 시스템 (Damage Text Pool)
- 히트 리액션 / 사망 연출 시스템
- 크로스헤어 / 히트마커 / HUD 시스템
- 결과 화면 / 게임 오버 시스템

---

## 🔧 Troubleshooting

| Issue | Solution |
|---|---|
| 생성자에서 CVar 접근 ensure 실패 | `StopAura()` 등 런타임 함수를 생성자에서 제거, `BeginPlay` 이후로 이동 |
| 히트스캔 비주얼 오브젝트에 재피격 | 스폰된 비주얼 액터의 충돌을 `NoCollision`으로 완전 비활성화 |
| 투사체 동일 몬스터 중복 데미지 | `HitActors` TSet으로 중복 방지 + `IgnoreActorWhenMoving` 적용 |
| HPBar 위젯 BeginPlay 이전 크래시 | `HasActorBegunPlay()` 가드 + `InitWidget()` 호출 보장 |
| 몬스터 간 물리 충돌로 밀림 | 몬스터 간 Overlap 허용, 군집 이동 자연스럽게 처리 |
| NPC 유지비 미지급 시 NPC 유실 | 밤 종료 시 축산물 부족 경고 → 다음 낮에도 미지급 시 자동 퇴직 |
