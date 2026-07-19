#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "PMBTTask_FindWaitLocation.generated.h"

UCLASS()
class PROJECTMETRUSH_API UPMBTTask_FindWaitLocation : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UPMBTTask_FindWaitLocation();

protected:
    /** 대기 위치를WaitLocation에 기록 */
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    // Variable
protected:
    /** 플레이어로부터의 대기 거리 -> 추후 조정필요 */
    UPROPERTY(EditAnywhere, Category = "Variable|AI")
    float WaitRadius = 300.f;

    /** 좌우 최대 각도 -> 추후 조정필요 */
    UPROPERTY(EditAnywhere, Category = "Variable|AI")
    float MaxSideAngle = 60.f;
};