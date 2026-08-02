#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Define/PMEnemyType.h"
#include "PMBTTask_FindHealTarget.generated.h"

UCLASS()
class PROJECTMETRUSH_API UPMBTTask_FindHealTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UPMBTTask_FindHealTarget();

protected:
	/** 회복 범위 안 부상 아군 중 HP 비율 최저 대상을 찾아 블랙보드에 기록 */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// Heal Method
private:
	/** HP 비율 동률일 때 쓰는 종류 우선순위 (작을수록 먼저) */
	int32 GetTypePriority(EPMEnemyType TargetType) const;

	// Heal Variable
protected:
	/** 회복 대상 탐색 범위 -> 추후 조정필요 : CastHeal의 HealRange와 같은 값을 유지해야 함 */
	UPROPERTY(EditAnywhere, Category = "Variable|Heal")
	float HealRange = 800.f;
};