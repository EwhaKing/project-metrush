#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "PMBTTask_GuardStance.generated.h"

UCLASS()
class PROJECTMETRUSH_API UPMBTTask_GuardStance : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UPMBTTask_GuardStance();

protected:
	/** 태스크 시작, 정면 방어 유지 진입 */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 매 틱 플레이어를 향해 느리게 회전, 밀치기 사거리/재평가 주기 도달 시 종료 */
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// Guard Variable
protected:
	/** 회전 속도 -> 추후 조정필요 */
	UPROPERTY(EditAnywhere, Category = "Variable|Guard")
	float RotationSpeed = 120.f;

	/** 밀치기로 넘어가는 거리 -> 추후 조정필요 : ShieldBash의 HitRange와 같은 값을 유지해야 함 */
	UPROPERTY(EditAnywhere, Category = "Variable|Guard")
	float BashRange = 220.f;

	/** 보호 위치를 다시 계산하기까지 유지 시간 -> 추후 조정필요 : 짧으면 재계산이 잦아 끊겨 보임 */
	UPROPERTY(EditAnywhere, Category = "Variable|Guard")
	float RecheckTime = 0.5f;

	// Runtime State
private:
	/** 태스크 시작 후 경과 시간 */
	float ElapsedTime = 0.f;
};