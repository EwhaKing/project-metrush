#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Define/PMAttackSlotType.h"
#include "PMBTTask_CastHeal.generated.h"

class APMEnemy;

UCLASS()
class PROJECTMETRUSH_API UPMBTTask_CastHeal : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UPMBTTask_CastHeal();

protected:
	/** 회복 조건 확인 후 슬롯 요청, 시전 시작 */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 시전 진행, 취소 조건 확인, 완료 시 회복 적용 */
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	/** 강제 중단 시 점등 복구, 슬롯 반환 */
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// Heal Method
private:
	/** 대상 최대 HP 비율만큼 회복 적용 */
	void ApplyHeal(UBehaviorTreeComponent& OwnerComp);

	/** 힐러와 대상을 잇는 연결선, 대상 위 회복 표시 */
	void DrawHealLink(UBehaviorTreeComponent& OwnerComp, APMEnemy* Healer, APMEnemy* Target) const;

	/** 시전 취소 -> 회복 미적용, 슬롯 반환 */
	void CancelCast(UBehaviorTreeComponent& OwnerComp, const TCHAR* Reason);

	/** 전신 점등 초기화 */
	void ResetTelegraph(UBehaviorTreeComponent& OwnerComp);

	/** 슬롯 반환 */
	void ReleaseHeldSlot(UBehaviorTreeComponent& OwnerComp);

	/** 블랙보드에 기록된 회복 대상 */
	APMEnemy* GetHealTarget(UBehaviorTreeComponent& OwnerComp) const;

	// Heal Variable
protected:
	/** 슬롯 종류 */
	UPROPERTY(EditAnywhere, Category = "Variable|Heal")
	EPMAttackSlotType SlotType = EPMAttackSlotType::Heal;

	/** 회복 시전 시간 -> 추후 조정필요 : 플레이어가 힐러를 끊으러 갈 수 있는 시간인지 검증 필요 */
	UPROPERTY(EditAnywhere, Category = "Variable|Heal")
	float CastTime = 1.2f;

	/** 회복 범위 -> 추후 조정필요 : FindHealTarget의 HealRange와 같은 값을 유지해야 함 */
	UPROPERTY(EditAnywhere, Category = "Variable|Heal")
	float HealRange = 800.f;

	/** 최대 HP 대비 회복 비율 -> 추후 조정필요 : 회복량 20% */
	UPROPERTY(EditAnywhere, Category = "Variable|Heal")
	float HealPercent = 0.2f;

	/** 추후 조정필요 : 텔레그래프 색 */
	UPROPERTY(EditAnywhere, Category = "Variable|Heal")
	FLinearColor TelegraphColor = FLinearColor::Green;

	/** 평상시 몸 색 */
	UPROPERTY(EditAnywhere, Category = "Variable|Heal")
	FLinearColor NormalColor = FLinearColor::White;

	// Runtime State
private:
	/** 시전 시작 후 경과 시간 */
	float ElapsedTime = 0.f;

	/** 슬롯 보유 여부 */
	bool bHasSlot = false;

	/** 시전 시작 시점의 피격 횟수 */
	int32 StartDamagedCount = 0;
};