#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "PMBTTask_FindGuardPosition.generated.h"

UCLASS()
class PROJECTMETRUSH_API UPMBTTask_FindGuardPosition : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UPMBTTask_FindGuardPosition();

protected:
	/** 보호 대상 탐색 후 플레이어와 대상 사이 위치 기록 */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// Guard Variable
protected:
	/** 보호 대상 탐색 반경 -> 추후 조정필요 */
	UPROPERTY(EditAnywhere, Category = "Variable|Guard")
	float SearchRadius = 1500.f;

	/** 보호 대상 앞으로 설 거리 -> 추후 조정필요 */
	UPROPERTY(EditAnywhere, Category = "Variable|Guard")
	float GuardStandoff = 250.f;
};