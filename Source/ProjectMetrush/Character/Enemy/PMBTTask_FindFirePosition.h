#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "PMBTTask_FindFirePosition.generated.h"

UCLASS()
class PROJECTMETRUSH_API UPMBTTask_FindFirePosition : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UPMBTTask_FindFirePosition();

protected:
    /** 유효 사거리와 시야가 확보되는 사격 위치를 FirePosition에 기록 */
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    // Variable
protected:
    /** 목표 사격 거리 -> 추후 조정필요 : RangedAttack의 MinRange~MaxRange 사이 값을 유지해야 함 */
    UPROPERTY(EditAnywhere, Category = "Variable|AI")
    float FireDistance = 850.f;
};