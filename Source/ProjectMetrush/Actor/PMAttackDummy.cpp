#include "ProjectMetrush/Actor/PMAttackDummy.h"
#include "ProjectMetrush/Component/PMCombatComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

APMAttackDummy::APMAttackDummy()
{
	PrimaryActorTick.bCanEverTick = true;

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	SetRootComponent(BodyMesh);
}

void APMAttackDummy::BeginPlay()
{
	Super::BeginPlay();
	Phase = EPMDummyPhase::Idle;
	PhaseElapsedTime = 0.f;
}

void APMAttackDummy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PhaseElapsedTime += DeltaTime;

	switch (Phase)
	{
	case EPMDummyPhase::Idle:
		if (PhaseElapsedTime >= IdleDuration)
		{
			Phase = EPMDummyPhase::Telegraph;
			PhaseElapsedTime = 0.f;
		}
		break;

	case EPMDummyPhase::Telegraph:
		if (PhaseElapsedTime >= TelegraphDuration)
		{
			// 텔레그래프 종료 = 공격 판정 발생 순간
			PerformAttack();
			Phase = EPMDummyPhase::Recover;
			PhaseElapsedTime = 0.f;
		}
		break;

	case EPMDummyPhase::Recover:
		if (PhaseElapsedTime >= RecoverDuration)
		{
			Phase = EPMDummyPhase::Idle;
			PhaseElapsedTime = 0.f;
		}
		break;
	}

	DrawPhaseDebug();
}

void APMAttackDummy::PerformAttack()
{
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!Player)
	{
		return;
	}

	// 전방 박스 범위 판정: 로컬 좌표로 변환해 검사
	const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	const float ForwardDist = FVector::DotProduct(ToPlayer, GetActorForwardVector());
	const float SideDist = FMath::Abs(FVector::DotProduct(ToPlayer, GetActorRightVector()));

	const bool bInRange = ForwardDist > 0.f && ForwardDist <= AttackRange && SideDist <= AttackWidth * 0.5f;
	if (!bInRange)
	{
		return;
	}

	// 플레이어 상태에 따른 결과 처리
	UPMCombatComponent* Combat = Player->FindComponentByClass<UPMCombatComponent>();
	if (Combat && Combat->IsInvincible())
	{
		const bool bJustDodge = Combat->GetCombatState() == EPMCombatState::Dodge;
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green,
				bJustDodge ? TEXT("JUST DODGE!") : TEXT("INVINCIBLE"));
		}
		// TODO: 저스트 회피 성공 시 강화 반격 윈도우 열기 (기획서 9장)
		return;
	}

	// TODO: GAS 도입 후 데미지 GE 적용으로 교체
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red,
			FString::Printf(TEXT("PLAYER HIT! (%.0f dmg)"), Damage));
	}
}

void APMAttackDummy::DrawPhaseDebug() const
{
	// 판정 박스의 중심: 전방으로 절반 거리
	const FVector BoxCenter = GetActorLocation() + GetActorForwardVector() * (AttackRange * 0.5f);
	const FVector BoxExtent(AttackRange * 0.5f, AttackWidth * 0.5f, 100.f);

	// 단계별 색: 대기=흰색, 예고=노랑→빨강으로 변화, 휴식=회색
	FColor Color = FColor::White;
	if (Phase == EPMDummyPhase::Telegraph)
	{
		const float Ratio = PhaseElapsedTime / TelegraphDuration;
		Color = Ratio > 0.7f ? FColor::Red : FColor::Yellow;
	}
	else if (Phase == EPMDummyPhase::Recover)
	{
		Color = FColor(128, 128, 128);
	}

	DrawDebugBox(GetWorld(), BoxCenter, BoxExtent, GetActorQuat(), Color, false, -1.f, 0, 2.f);
}