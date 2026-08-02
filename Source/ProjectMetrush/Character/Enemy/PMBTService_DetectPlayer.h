#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "PMBTService_DetectPlayer.generated.h"

UCLASS()
class PROJECTMETRUSH_API UPMBTService_DetectPlayer : public UBTService
{
    GENERATED_BODY()

public:
    UPMBTService_DetectPlayer();

protected:
    /** 인터벌마다 플레이어 인지, 추격 종료 수행 */
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    // Variable
protected:
    /** 플레이어 인지 범위 -> 추후 조정필요 : 전투 방 크기 확정 후 조정, 시야각 없이 거리로만 인지 중 */
    UPROPERTY(EditAnywhere, Category = "Variable|AI")
    float DetectRadius = 1000.0f;

    /** 시야 잃은 후 추격 종료 시간 -> 추후 조정필요 */
    UPROPERTY(EditAnywhere, Category = "Variable|AI")
    float LoseSightTime = 3.0f;

    /** 시작 위치 기준 추격 종료 거리 -> 추후 조정필요 : 방을 벗어나 따라오지 않도록 방 크기와 맞춰야 함 */
    UPROPERTY(EditAnywhere, Category = "Variable|AI")
    float MaxChaseDistance = 2000.0f;

    /** 경로 탐색 실패 시간 -> 추후 조정필요 */
    UPROPERTY(EditAnywhere, Category = "Variable|AI")
    float PathFailTime = 2.0f;

    // Runtime State
private:
    /** 플레이어를 마지막으로 본 시간 */
    float LastSeenTime = 0.0f;

    /** 경로 탐색 실패 시작 시간 */
    float PathFailStartTime = -1.0f;
};