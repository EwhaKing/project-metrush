#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PMCombatComponent.generated.h"

class ACharacter;

/** 전투 상태. Idle/Move는 전투 행동이 없는 상태이므로 None 하나로 취급합니다. */
UENUM(BlueprintType)
enum class EPMCombatState : uint8
{
	None,	// 대기/이동 (자유 상태)
	Attack,	// 기본 공격 진행 중 (콤보 단계는 ComboIndex로 구분)
	Dodge,	// 회피 진행 중
	Hit,	// 피격 경직 (미구현, 자리만)
	Dead	// 사망 (미구현, 자리만)
};

/** 기본 공격 한 타의 수치 묶음입니다. 기획서 7.2, 7.3 */
USTRUCT(BlueprintType)
struct FPMAttackData
{
	GENERATED_BODY()

	/** 공격 총 길이(초) */
	UPROPERTY(EditDefaultsOnly)
	float Duration = 0.38f;

	/** 히트 판정 시작 시점(초) */
	UPROPERTY(EditDefaultsOnly)
	float ActiveStart = 0.08f;

	/** 히트 판정 종료 시점(초). 이후부터 회피/패링 캔슬 가능(임시 기준) */
	UPROPERTY(EditDefaultsOnly)
	float ActiveEnd = 0.14f;

	/** 피해량 */
	UPROPERTY(EditDefaultsOnly)
	float Damage = 10.f;

	/** 공격 중 전진 거리(cm) */
	UPROPERTY(EditDefaultsOnly)
	float AdvanceDistance = 30.f;

	/** 판정 전방 거리(cm) */
	UPROPERTY(EditDefaultsOnly)
	float Range = 180.f;

	/** 판정 좌우 반각(도). 기획서의 좌우 각도 100도 = 반각 50도 */
	UPROPERTY(EditDefaultsOnly)
	float HalfAngleDeg = 50.f;

	/** 이 시점(초)부터 회피/패링 캔슬이 가능합니다. 기획서 7.4의 "후반부" */
	UPROPERTY(EditDefaultsOnly)
	float CancelWindowStart = 0.14f;
};

/** 플레이어 전투 상태 머신과 액션(공격, 회피) 실행을 담당하는 컴포넌트입니다. */
UCLASS()
class PROJECTMETRUSH_API UPMCombatComponent : public UActorComponent
{
	GENERATED_BODY()

	// Component Interface
public:
	UPMCombatComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Combat Method
public:
	/** 공격 입력을 처리합니다. 상태에 따라 즉시 발동, 콤보 예약, 무시 중 하나로 처리됩니다. */
	void TryAttack(const FVector& FacingDirection);

	/** 회피 입력을 처리합니다. 캔슬 규칙(기획서 5.5)을 통과해야 발동합니다. */
	void TryDodge(const FVector& DodgeDirection);

	/** 이동 입력을 받아도 되는 상태인지 반환합니다. */
	bool CanMove() const { return CombatState == EPMCombatState::None; }

	/** 현재 무적 상태인지 반환합니다. (회피 무적 + 피격 후 무적) */
	bool IsInvincible() const { return bIsInvincible || HitInvincibleRemaining > 0.f; }

	/** 현재 전투 상태를 반환합니다. */
	EPMCombatState GetCombatState() const { return CombatState; }

	/** 저스트 회피 성공을 접수합니다. 회피 계열 증강(A013/A014/A016)이 여기서 발동합니다. */
	void NotifyJustDodge();

	/** 피격 처리를 시작합니다. 경직 상태 진입과 피격 후 무적을 포함합니다. 기획서 10장 */
	void EnterHit(float StaggerDuration);

	/** 사망 상태로 전환합니다. 모든 행동이 차단됩니다. */
	void EnterDead();

private:
	void StartAttack(int32 NewComboIndex, const FVector& FacingDirection);
	void UpdateAttack(float DeltaTime);
	void EndAttack();
	/** 주변 적을 감속시킵니다. A013 완벽 회피. */
	void ApplyEnemySlow(float SlowAmount);

	/** 현재 타의 부채꼴 범위 안 캐릭터를 찾아 명중 처리합니다. 기획서 7.3 */
	void PerformAttackTrace();

	bool CanDodge() const;
	void StartDodge(const FVector& DodgeDirection);
	void UpdateDodge(float DeltaTime);
	void EndDodge();
	void UpdateHit(float DeltaTime);

	// Attack Variable
protected:
	/** 기본 공격 1~3타의 수치입니다. 순서 = 콤보 순서. 기획서 7.2, 7.3 */
	UPROPERTY(EditDefaultsOnly, Category = "변수|공격")
	TArray<FPMAttackData> AttackDataList;

	/** 공격 종료 후 콤보가 유지되는 시간(초)입니다. 기획서 7.1 */
	UPROPERTY(EditDefaultsOnly, Category = "변수|공격")
	float ComboKeepTime = 0.3f;

	// Dodge Variable
protected:
	/** 회피 총 길이(초)입니다. 기획서 5.3 */
	UPROPERTY(EditDefaultsOnly, Category = "변수|회피")
	float DodgeDuration = 0.4f;

	/** 회피 이동 거리(cm)입니다. 기획서 5.3 */
	UPROPERTY(EditDefaultsOnly, Category = "변수|회피")
	float DodgeDistance = 260.f;

	/** 회피 시작 후 무적 지속 시간(초)입니다. 기획서 5.3 */
	UPROPERTY(EditDefaultsOnly, Category = "변수|회피")
	float DodgeInvincibleDuration = 0.2f;

	/** 연속 사용 가능한 회피 횟수입니다. 기획서 5.3 */
	UPROPERTY(EditDefaultsOnly, Category = "변수|회피")
	int32 MaxDodgeCount = 2;

	/** 연속 회피 소진 후 쿨타임(초)입니다. 기획서 5.3 */
	UPROPERTY(EditDefaultsOnly, Category = "변수|회피")
	float DodgeCooldown = 0.6f;

	// Hit Variable
protected:
	/** 소피격 경직 시간(초)입니다. 기획서 10.1 */
	UPROPERTY(EditDefaultsOnly, Category = "변수|피격")
	float HitStaggerDuration = 0.1f;

	/** 피격 직후 무적 시간(초)입니다. 연속 피격 즉사 방지. 기획서 10.3 */
	UPROPERTY(EditDefaultsOnly, Category = "변수|피격")
	float HitInvincibleDuration = 0.3f;

	// Augment Variable
protected:
	/** A013 완벽 회피의 적 감속 지속 시간(초)입니다. 기획서: 1단계 1초 (단계별 시간 차이는 추후) */
	UPROPERTY(EditDefaultsOnly, Category = "변수|증강")
	float EnemySlowDuration = 1.f;

	/** A014 공격적 회피의 강화 유효 시간(초)입니다. 기획서: 저스트 후 3초 내 공격 */
	UPROPERTY(EditDefaultsOnly, Category = "변수|증강")
	float JustDodgeBuffWindow = 3.f;

private:
	/** A014: 저스트 회피 후 첫 공격 강화가 대기 중인가 */
	bool bJustDodgeAttackReady = false;

	/** A014: 강화 대기가 만료되는 시각 */
	float JustDodgeBuffExpireTime = 0.f;

	/** A013: 현재 감속이 풀리는 시각 (S005 시너지 판정용) */
	float EnemySlowEndTime = 0.f;

	/** A013: 감속 해제 타이머 */
	FTimerHandle EnemySlowTimerHandle;

private:
	/** 현재 피격의 경직 길이 */
	float CurrentHitStagger = 0.1f;

	/** 피격 후 무적의 남은 시간 */
	float HitInvincibleRemaining = 0.f;

private:
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	EPMCombatState CombatState = EPMCombatState::None;

	/** 현재 상태에 진입한 뒤 흐른 시간(초) */
	float StateElapsedTime = 0.f;

	// 공격 진행 상태
	int32 ComboIndex = 0;			// 현재 몇 타인지 (1~3, 0이면 공격 아님)
	bool bAttackBuffered = false;	// 공격 중 다음 타 입력이 예약되었는가
	bool bHitApplied = false;		// 이번 타의 판정을 이미 수행했는가
	float LastAttackEndTime = -100.f;
	int32 LastComboIndex = 0;
	FVector AttackDirection = FVector::ForwardVector;

	// 회피 진행 상태
	bool bIsInvincible = false;
	bool bDodgeOnCooldown = false;
	int32 CurrentDodgeCount = 0;
	FVector DodgeMoveDirection = FVector::ForwardVector;
	FTimerHandle DodgeCooldownTimerHandle;
};