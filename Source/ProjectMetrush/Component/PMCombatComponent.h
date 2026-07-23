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

	/** 현재 무적 상태인지 반환합니다. (추후 피격 판정에서 사용) */
	bool IsInvincible() const { return bIsInvincible; }

	/** 현재 전투 상태를 반환합니다. */
	EPMCombatState GetCombatState() const { return CombatState; }

private:
	void StartAttack(int32 NewComboIndex, const FVector& FacingDirection);
	void UpdateAttack(float DeltaTime);
	void EndAttack();

	/** 현재 타의 부채꼴 범위 안 캐릭터를 찾아 명중 처리합니다. 기획서 7.3 */
	void PerformAttackTrace();

	bool CanDodge() const;
	void StartDodge(const FVector& DodgeDirection);
	void UpdateDodge(float DeltaTime);
	void EndDodge();

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