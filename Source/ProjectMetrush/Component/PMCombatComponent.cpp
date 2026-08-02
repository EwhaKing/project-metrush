#include "ProjectMetrush/Component/PMCombatComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectMetrush/Character/Player/PMPlayer.h"
#include "ProjectMetrush/Component/PMStatComponent.h"

UPMCombatComponent::UPMCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 기획서 7.2, 7.3의 1~3타 기준값
	FPMAttackData Attack1;
	Attack1.Duration = 0.38f; Attack1.ActiveStart = 0.08f; Attack1.ActiveEnd = 0.14f;
	Attack1.Damage = 10.f; Attack1.AdvanceDistance = 30.f; Attack1.Range = 180.f; Attack1.HalfAngleDeg = 50.f;
	Attack1.CancelWindowStart = 0.14f;

	FPMAttackData Attack2;
	Attack2.Duration = 0.42f; Attack2.ActiveStart = 0.10f; Attack2.ActiveEnd = 0.17f;
	Attack2.Damage = 12.f; Attack2.AdvanceDistance = 35.f; Attack2.Range = 190.f; Attack2.HalfAngleDeg = 50.f;
	Attack2.CancelWindowStart = 0.17f;

	FPMAttackData Attack3;
	Attack3.Duration = 0.58f; Attack3.ActiveStart = 0.19f; Attack3.ActiveEnd = 0.26f;
	Attack3.Damage = 18.f; Attack3.AdvanceDistance = 45.f; Attack3.Range = 220.f; Attack3.HalfAngleDeg = 65.f;
	Attack3.CancelWindowStart = 0.26f;

	AttackDataList = { Attack1, Attack2, Attack3 };
}

void UPMCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void UPMCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	StateElapsedTime += DeltaTime;
	if (HitInvincibleRemaining > 0.f)
	{
		HitInvincibleRemaining -= DeltaTime;
	}

	switch (CombatState)
	{
	case EPMCombatState::Attack:
		UpdateAttack(DeltaTime);
		break;
	case EPMCombatState::Dodge:
		UpdateDodge(DeltaTime);
		break;
	case EPMCombatState::Hit:
		UpdateHit(DeltaTime);
		break;
	default:
		break;
	}
}

// 공격 -------------------------------------------------------------------

void UPMCombatComponent::TryAttack(const FVector& FacingDirection)
{
	if (CombatState == EPMCombatState::None)
	{
		// 콤보 유지 시간 내 재입력이면 다음 타, 아니면 1타부터 (기획서 7.1)
		const float Now = GetWorld()->GetTimeSeconds();
		const bool bKeepCombo = (Now - LastAttackEndTime <= ComboKeepTime) && (LastComboIndex < AttackDataList.Num());
		StartAttack(bKeepCombo ? LastComboIndex + 1 : 1, FacingDirection);
	}
	else if (CombatState == EPMCombatState::Attack)
	{
		// 공격 중 입력은 예약해뒀다가 현재 타가 끝나면 이어감 (입력 버퍼)
		if (ComboIndex < AttackDataList.Num())
		{
			bAttackBuffered = true;
		}
	}
	// Dodge 중 공격 입력은 반격 시스템(기획서 9장)에서 처리 예정. 지금은 무시.
}

void UPMCombatComponent::StartAttack(int32 NewComboIndex, const FVector& FacingDirection)
{
	CombatState = EPMCombatState::Attack;
	StateElapsedTime = 0.f;
	ComboIndex = NewComboIndex;
	bAttackBuffered = false;
	bHitApplied = false;

	// 무방향 공격은 마지막 유효 이동 방향 사용 (기획서 7.1)
	AttackDirection = FacingDirection.GetSafeNormal();
	OwnerCharacter->SetActorRotation(AttackDirection.Rotation());

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.4f, FColor::Yellow,
			FString::Printf(TEXT("ATTACK %d"), ComboIndex));
	}
}

void UPMCombatComponent::UpdateAttack(float DeltaTime)
{
	const FPMAttackData& Data = AttackDataList[ComboIndex - 1];

	// 공격 중 전진: 전진 거리를 공격 길이에 균등 분배 (기획서 7.2)
	UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
	const float AdvanceSpeed = Data.AdvanceDistance / Data.Duration;
	MoveComp->Velocity = AttackDirection * AdvanceSpeed + FVector(0.f, 0.f, MoveComp->Velocity.Z);

	// 활성 시간 진입 시 1회 판정 (기획서 7.2 활성 시간)
	if (!bHitApplied && StateElapsedTime >= Data.ActiveStart)
	{
		PerformAttackTrace();
		bHitApplied = true;
	}

	if (StateElapsedTime >= Data.Duration)
	{
		EndAttack();
	}
}

void UPMCombatComponent::EndAttack()
{
	LastAttackEndTime = GetWorld()->GetTimeSeconds();
	LastComboIndex = ComboIndex;

	const bool bChainNext = bAttackBuffered && (ComboIndex < AttackDataList.Num());
	const FVector KeepDirection = AttackDirection;

	CombatState = EPMCombatState::None;
	StateElapsedTime = 0.f;
	ComboIndex = 0;
	bAttackBuffered = false;

	OwnerCharacter->GetCharacterMovement()->Velocity = FVector::ZeroVector;

	if (bChainNext)
	{
		StartAttack(LastComboIndex + 1, KeepDirection);
	}
}

void UPMCombatComponent::PerformAttackTrace()
{
	const FPMAttackData& Data = AttackDataList[ComboIndex - 1];
	const FVector Origin = OwnerCharacter->GetActorLocation();

	// 판정 범위 시각화 (몬스터 구현 전 임시)
	DrawDebugCone(GetWorld(), Origin, AttackDirection, Data.Range,
		FMath::DegreesToRadians(Data.HalfAngleDeg), FMath::DegreesToRadians(30.f),
		12, FColor::Red, false, 0.25f);

	// 범위 내 캐릭터 탐색: 구 반경으로 거른 뒤 각도 검사 (기획서 7.3 부채꼴)
	TArray<AActor*> Overlapped;
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), Origin, Data.Range,
		{ UEngineTypes::ConvertToObjectType(ECC_Pawn) }, ACharacter::StaticClass(),
		{ OwnerCharacter }, Overlapped);

	for (AActor* Target : Overlapped)
	{
		const FVector ToTarget = (Target->GetActorLocation() - Origin).GetSafeNormal2D();
		const float CosAngle = FVector::DotProduct(AttackDirection, ToTarget);

		if (CosAngle >= FMath::Cos(FMath::DegreesToRadians(Data.HalfAngleDeg)))
		{
			float FinalDamage = Data.Damage;

			const UPMStatComponent* Stat = OwnerCharacter->FindComponentByClass<UPMStatComponent>();
			if (Stat)
			{
				// A004 마지막 일격: 콤보 마지막 타(3타)의 피해 증가
				if (ComboIndex == AttackDataList.Num())
				{
					FinalDamage *= 1.f + Stat->GetEffectValue(EPMAugmentEffect::FinalHitDamage);
				}

				// A035 혈투: HP 50% 이하부터 잃은 체력에 선형 비례해 피해 증가 (기획서 14.7)
				if (const APMPlayer* Player = Cast<APMPlayer>(OwnerCharacter))
				{
					const float HPRatio = Player->GetHPRatio();
					if (HPRatio < 0.5f)
					{
						const float MaxBonus = Stat->GetEffectValue(EPMAugmentEffect::LowHPDamage);
						FinalDamage *= 1.f + MaxBonus * (1.f - HPRatio / 0.5f);
					}
				}

				// A014 공격적 회피: 저스트 회피 후 첫 공격 강화 (1회 소비)
				if (bJustDodgeAttackReady)
				{
					if (GetWorld()->GetTimeSeconds() <= JustDodgeBuffExpireTime)
					{
						float BuffValue = Stat->GetEffectValue(EPMAugmentEffect::JustDodgeFirstAttack);

						// S005 시간 반격: A013 감속이 유지되는 동안이면 1.5배 (기획서 15장)
						const bool bSynergy = Stat->HasEffect(EPMAugmentEffect::JustDodgeEnemySlow)
							&& GetWorld()->GetTimeSeconds() <= EnemySlowEndTime;
						if (bSynergy)
						{
							BuffValue *= 1.5f;
							if (GEngine)
							{
								GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Magenta, TEXT("시너지: 시간 반격!"));
							}
						}

						FinalDamage *= 1.f + BuffValue;
					}
					bJustDodgeAttackReady = false; // 만료됐든 적중했든 1회로 소멸
				}
			}

			UGameplayStatics::ApplyDamage(Target, FinalDamage,
				OwnerCharacter->GetController(), OwnerCharacter, nullptr);
		}
	}
}

// 회피 -------------------------------------------------------------------

void UPMCombatComponent::TryDodge(const FVector& DodgeDirection)
{
	if (CanDodge())
	{
		StartDodge(DodgeDirection);
	}
}

bool UPMCombatComponent::CanDodge() const
{
	if (bDodgeOnCooldown)
	{
		return false;
	}

	switch (CombatState)
	{
	case EPMCombatState::None:
		return true;

	case EPMCombatState::Attack:
	{
		// 1, 2타는 후반부(판정 종료 후) 캔슬 가능, 3타는 불가 (기획서 5.5)
		if (ComboIndex >= AttackDataList.Num())
		{
			return false;
		}
		return StateElapsedTime >= AttackDataList[ComboIndex - 1].CancelWindowStart;
	}

	default:	// Dodge, Hit, Dead
		return false;
	}
}

void UPMCombatComponent::StartDodge(const FVector& DodgeDirection)
{
	CombatState = EPMCombatState::Dodge;
	StateElapsedTime = 0.f;
	ComboIndex = 0;
	bAttackBuffered = false;

	bIsInvincible = true;
	++CurrentDodgeCount;

	DodgeMoveDirection = DodgeDirection.GetSafeNormal();
	OwnerCharacter->SetActorRotation(DodgeMoveDirection.Rotation());

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Cyan,
			FString::Printf(TEXT("DODGE %d/%d"), CurrentDodgeCount, MaxDodgeCount));
	}
}

void UPMCombatComponent::UpdateDodge(float DeltaTime)
{
	if (bIsInvincible && StateElapsedTime >= DodgeInvincibleDuration)
	{
		bIsInvincible = false;
	}

	UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
	const float DodgeSpeed = DodgeDistance / DodgeDuration;
	MoveComp->Velocity = DodgeMoveDirection * DodgeSpeed + FVector(0.f, 0.f, MoveComp->Velocity.Z);

	if (StateElapsedTime >= DodgeDuration)
	{
		EndDodge();
	}
}

void UPMCombatComponent::EndDodge()
{
	CombatState = EPMCombatState::None;
	StateElapsedTime = 0.f;
	bIsInvincible = false;

	UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
	MoveComp->Velocity = FVector(0.f, 0.f, MoveComp->Velocity.Z);

	if (CurrentDodgeCount >= MaxDodgeCount)
	{
		bDodgeOnCooldown = true;
		GetWorld()->GetTimerManager().SetTimer(DodgeCooldownTimerHandle, [this]()
			{
				bDodgeOnCooldown = false;
				CurrentDodgeCount = 0;
			}, DodgeCooldown, false);
	}
}

// 피격/사망 ---------------------------------------------------------------

void UPMCombatComponent::EnterHit(float StaggerDuration)
{
	if (CombatState == EPMCombatState::Dead)
	{
		return;
	}

	// 진행 중이던 공격/회피를 중단하고 피격 상태로 (기획서 11.2: 피격이 행동보다 우선)
	CombatState = EPMCombatState::Hit;
	StateElapsedTime = 0.f;
	ComboIndex = 0;
	bAttackBuffered = false;
	bIsInvincible = false;

	CurrentHitStagger = StaggerDuration;
	HitInvincibleRemaining = HitInvincibleDuration;

	// 피격 순간 이동 정지
	OwnerCharacter->GetCharacterMovement()->Velocity = FVector::ZeroVector;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Orange, TEXT("PLAYER HIT"));
	}
}

void UPMCombatComponent::UpdateHit(float DeltaTime)
{
	// 경직이 끝나면 자유 상태로 복귀
	if (StateElapsedTime >= CurrentHitStagger)
	{
		CombatState = EPMCombatState::None;
		StateElapsedTime = 0.f;
	}
}

void UPMCombatComponent::EnterDead()
{
	CombatState = EPMCombatState::Dead;
	StateElapsedTime = 0.f;
	bIsInvincible = false;

	OwnerCharacter->GetCharacterMovement()->StopMovementImmediately();
	OwnerCharacter->GetCharacterMovement()->DisableMovement();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("PLAYER DEAD"));
	}
}

// 증강 -------------------------------------------------------------------

void UPMCombatComponent::NotifyJustDodge()
{
	const UPMStatComponent* Stat = OwnerCharacter->FindComponentByClass<UPMStatComponent>();
	if (!Stat)
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();

	// A016 회피 충전: 쿨타임 중이면 즉시 해제 (프로토타입 단순화: % 회복 대신 전액 회복)
	if (Stat->HasEffect(EPMAugmentEffect::JustDodgeCooldownRefund) && bDodgeOnCooldown)
	{
		GetWorld()->GetTimerManager().ClearTimer(DodgeCooldownTimerHandle);
		bDodgeOnCooldown = false;
		CurrentDodgeCount = 0;
	}

	// A014 공격적 회피: 다음 첫 공격 강화 예약 (3초 유효)
	if (Stat->HasEffect(EPMAugmentEffect::JustDodgeFirstAttack))
	{
		bJustDodgeAttackReady = true;
		JustDodgeBuffExpireTime = Now + JustDodgeBuffWindow;
	}

	// A013 완벽 회피: 주변 적 감속
	const float SlowAmount = Stat->GetEffectValue(EPMAugmentEffect::JustDodgeEnemySlow);
	if (SlowAmount > 0.f)
	{
		ApplyEnemySlow(SlowAmount);
		EnemySlowEndTime = Now + EnemySlowDuration;
	}
}

void UPMCombatComponent::ApplyEnemySlow(float SlowAmount)
{
	// 화면 부근의 모든 적을 감속. CustomTimeDilation = 액터 개별 시간 배속 (0.7 = 30% 감속)
	TArray<AActor*> Enemies;
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), OwnerCharacter->GetActorLocation(), 1500.f,
		{ UEngineTypes::ConvertToObjectType(ECC_Pawn) }, ACharacter::StaticClass(),
		{ OwnerCharacter }, Enemies);

	for (AActor* Enemy : Enemies)
	{
		Enemy->CustomTimeDilation = 1.f - SlowAmount;
	}

	// 지속 시간 후 원복. 새 저스트가 오면 타이머가 갱신되어 연장됨
	FTimerDelegate RestoreDelegate;
	RestoreDelegate.BindLambda([this]()
		{
			TArray<AActor*> AllEnemies;
			UKismetSystemLibrary::SphereOverlapActors(GetWorld(), OwnerCharacter->GetActorLocation(), 3000.f,
				{ UEngineTypes::ConvertToObjectType(ECC_Pawn) }, ACharacter::StaticClass(),
				{ OwnerCharacter }, AllEnemies);
			for (AActor* Enemy : AllEnemies)
			{
				Enemy->CustomTimeDilation = 1.f;
			}
		});
	GetWorld()->GetTimerManager().SetTimer(EnemySlowTimerHandle, RestoreDelegate, EnemySlowDuration, false);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Magenta,
			FString::Printf(TEXT("완벽 회피! 적 %d체 감속"), Enemies.Num()));
	}
}