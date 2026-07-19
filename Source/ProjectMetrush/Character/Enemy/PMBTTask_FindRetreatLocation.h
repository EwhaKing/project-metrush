#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "PMBTTask_FindRetreatLocation.generated.h"

UCLASS()
class PROJECTMETRUSH_API UPMBTTask_FindRetreatLocation : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UPMBTTask_FindRetreatLocation();

protected:
    /** 플레이어가 후퇴 거리 안이면 반대 방향 후퇴 지점을 찾아 기록, 밖이면 후퇴 x */
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    // Variable
protected:
    /** 후퇴 시작 거리 */
    UPROPERTY(EditAnywhere, Category = "Variable|AI")
    float RetreatTriggerRange = 300.f;

    /** 후방 이동 거리 */
    UPROPERTY(EditAnywhere, Category = "Variable|AI")
    float RetreatDistance = 250.f;
};