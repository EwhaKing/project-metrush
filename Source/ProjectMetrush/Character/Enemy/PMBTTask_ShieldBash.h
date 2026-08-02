#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Define/PMAttackSlotType.h"
#include "PMBTTask_ShieldBash.generated.h"

UCLASS()
class PROJECTMETRUSH_API UPMBTTask_ShieldBash : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UPMBTTask_ShieldBash();

protected:
	/** 태스크 시작 -> 거리, 슬롯 확인 후 준비 점등 */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 준비 -> 판정 -> 후딜레이 */
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	/** 태스크 강제 중단 시 점등 복구, 슬롯 반환 */
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// Attack Method
private:
	/** 밀치기 판정 1회, 명중 시 플레이어 넉백, 피해 */
	void DoBash(UBehaviorTreeComponent& OwnerComp);

	/** 전신 점등 초기화 */
	void ResetTelegraph(UBehaviorTreeComponent& OwnerComp);

	/** 슬롯 반환 */
	void ReleaseHeldSlot(UBehaviorTreeComponent& OwnerComp);

	// Attack Variable
protected:
	/** 슬롯 종류 */
	UPROPERTY(EditAnywhere, Category = "Variable|Attack")
	EPMAttackSlotType SlotType = EPMAttackSlotType::ShieldBash;

	/** 밀치기 준비 시간 -> 추후 조정필요 : 플레이어 패링/회피 입력 창과 맞춰야 함 */
	UPROPERTY(EditAnywhere, Category = "Variable|Attack")
	float PrepareTime = 0.65f;

	/** 밀치기 후 다음 행동까지 후딜레이 -> 추후 조정필요 : 후방 공격을 노릴 수 있는 시간 */
	UPROPERTY(EditAnywhere, Category = "Variable|Attack")
	float AfterDelayTime = 1.0f;

	/** 밀치기 판정 거리 -> 추후 조정필요 : GuardStance의 BashRange와 같은 값을 유지해야 함 */
	UPROPERTY(EditAnywhere, Category = "Variable|Attack")
	float HitRange = 220.f;

	/** 텔레그래프 색 -> 추후 조정필요 */
	UPROPERTY(EditAnywhere, Category = "Variable|Attack")
	FLinearColor TelegraphColor = FLinearColor::Blue;

	/** 평상시 몸 색 */
	UPROPERTY(EditAnywhere, Category = "Variable|Attack")
	FLinearColor NormalColor = FLinearColor::White;

	// Runtime State
private:
	/** 태스크 시작 후 경과 시간 */
	float ElapsedTime = 0.f;

	/** 이번 밀치기에서 판정을 이미 수행했는지 */
	bool bHitChecked = false;

	/** 슬롯 보유 여부 */
	bool bHasSlot = false;
};