#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Define/PMAttackSlotType.h"
#include "PMBTTask_MeleeAttack.generated.h"

UCLASS()
class PROJECTMETRUSH_API UPMBTTask_MeleeAttack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UPMBTTask_MeleeAttack();

protected:
    /** 태스크 시작, 텔레그래프를 켜고 준비 단계 */
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    /** 준비 -> 판정 -> 후딜레이 */
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    /** 태스크가 강제 중단 시 점등 색 복구 */
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    // Attack Method
private:
    /** 공격 판정을 1회, 명중 시 플레이어 피격 스텁 */
    void DoHitCheck(UBehaviorTreeComponent& OwnerComp);

    /** 전신 점등 초기화 */
    void ResetTelegraph(UBehaviorTreeComponent& OwnerComp);

    /** 슬롯 반환 */
    void ReleaseHeldSlot(UBehaviorTreeComponent& OwnerComp);

    // Attack Variable
protected:
    /** 슬롯 종류 */
    UPROPERTY(EditAnywhere, Category = "Variable|Attack")
    EPMAttackSlotType SlotType = EPMAttackSlotType::Melee;

    /** 텔레그래프 시간 */
    UPROPERTY(EditAnywhere, Category = "Variable|Attack")
    float PrepareTime = 0.5f;

    /** 공격 판정 종료 후 다음 행동까지 */
    UPROPERTY(EditAnywhere, Category = "Variable|Attack")
    float AfterDelayTime = 0.7f;

    /** 공격 판정 거리 */
    UPROPERTY(EditAnywhere, Category = "Variable|Attack")
    float HitRange = 180.f;

    /** 텔레그래프 색 -> 추후 조정필요 */
    UPROPERTY(EditAnywhere, Category = "Variable|Attack")
    FLinearColor TelegraphColor = FLinearColor::Red;

    /** 평상시 몸 색 */
    UPROPERTY(EditAnywhere, Category = "Variable|Attack")
    FLinearColor NormalColor = FLinearColor::White;

    // Runtime State
private:
    /** 태스크 시작 후 경과 시간 */
    float ElapsedTime = 0.f;

    /** 이번 공격에서 판정을 이미 수행했는지 */
    bool bHitChecked = false;

    /** 슬롯 보유 여부 */
    bool bHasSlot = false;
};