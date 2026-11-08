# Magicult

Unreal Engine 5.3 기반 멀티플레이어 협동 액션 게임. 플레이어가 마법의 **방향 · 속도 · 속성 · 탄종 · 효과**를 실시간으로 조합해 발사하는 커스터마이즈 마법 시스템이 핵심입니다.

개인 포트폴리오 프로젝트이며, C++ 208개 파일 / 약 19,000줄로 구성돼 있습니다.

## 기술 스택

| 영역 | 사용 기술 |
|---|---|
| 엔진 | Unreal Engine 5.3 |
| 전투 | Gameplay Ability System (GAS) |
| 멀티플레이 | OnlineSubsystemSteam (리슨 서버) |
| 입력 | Enhanced Input |
| AI | Behavior Tree + 커스텀 패턴 선택 컴포넌트 |
| 기타 | Motion Warping, UMG + ViewModel |

## 구조

전투는 3개의 `WorldSubsystem`이 역할을 나눠 처리합니다.

- **`CombatSubsystem`** — 트리거 이벤트 수집과 큐 관리
- **`CombatResolveSubsystem`** — 원소 상호작용 판정 등 결과 확정
- **`ActionExecutorSubsystem`** — 확정된 액션 실행 (프레임 예산 기반)

공격은 `AttackInstance` 계층(근접 / 범위 / 투사체 / 오버랩)으로 추상화돼 있고, 각 인스턴스는 `DataAsset` 설정을 받아 동작합니다. 보스 AI는 패턴 **선택**(`BTTask_SelectBossPattern`)과 **실행**(`BTTask_ExecuteSelectedPattern`)을 분리해 Behavior Tree에서 조합합니다.

```
Source/Magicult/
├── AbilitySystem/   GA, GameplayEffect, ExecCalc
├── Attack/          AttackInstance 계층 + 설정 DataAsset
├── AI/Boss/         보스 패턴 선택·실행 BT 노드
├── Subsystem/       전투 파이프라인 3종
├── Character/       캐릭터 클래스 계층
├── UI/              HUD · Widget · WidgetController · ViewModel
└── Game/            GameMode, 매치 상태
```

서버 권위 분리, 복제 정책, 원소 상호작용 판정 등 설계 문서는 별도로 관리하고 있습니다.

## 빌드

Visual Studio 2022에서 `Magicult.sln`을 열고 빌드하거나, Unreal Editor에서 컴파일합니다.

```
UnrealBuildTool.exe Magicult Win64 Development "<프로젝트 경로>\Magicult.uproject"
```

> **주의 — 이 저장소만으로는 프로젝트가 실행되지 않습니다.**
> 서드파티 유료 에셋팩(`Content/Assets/`)은 라이선스상 저장소에 포함하지 않았습니다.
> 코드와 블루프린트 구조를 확인하는 용도로 봐주세요.

멀티플레이는 **리슨 서버** 구성만 대상으로 하며, 검증은 PIE 2인(Network Emulation 적용) 환경에서 진행했습니다. 데디케이티드 서버는 지원 목표가 아닙니다.

## 라이선스

코드(`Source/`)는 개인 포트폴리오 목적으로 공개합니다.
`Content/` 아래 Epic Games 제공 에셋(Mannequin, Paragon 등)은 각 에셋의 Epic Content License를 따릅니다.
