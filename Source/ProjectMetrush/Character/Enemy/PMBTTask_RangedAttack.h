#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Define/PMAttackSlotType.h"
#include "PMBTTask_RangedAttack.generated.h"

class APMProjectile;

UCLASS()
class PROJECTMETRUSH_API UPMBTTask_RangedAttack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UPMBTTask_RangedAttack();

protected:
    /** 사거리/시야/슬롯 확인 후 조준 */
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    /** 조준 -> 발사 -> 재장전 -> 슬롯 반환 -> 종료 */
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    /** 경직/타겟 상실로 중단 시 점등 복구, 슬롯 반환 */
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    // Attack Method
private:
    /** 플레이어 방향으로 투사체 생성 */
    void FireProjectile(UBehaviorTreeComponent& OwnerComp);

    /** 전신 점등 복구 */
    void ResetTelegraph(UBehaviorTreeComponent& OwnerComp);

    /** 슬롯 반환 */
    void ReleaseHeldSlot(UBehaviorTreeComponent& OwnerComp);

    // Attack Variable
protected:
    /** 슬롯 종류 */
    UPROPERTY(EditAnywhere, Category = "Variable|Attack")
    EPMAttackSlotType SlotType = EPMAttackSlotType::Ranged;

    /** 조준 시간 */
    UPROPERTY(EditAnywhere, Category = "Variable|Attack")
    float AimTime = 0.8f;

    /** 재장전 시간 */
    UPROPERTY(EditAnywhere, Category = "Variable|Attack")
    float ReloadTime = 1.f;

    /** 유효 사거리 최소 */
    UPROPERTY(EditAnywhere, Category = "Variable|Attack")
    float MinRange = 600.f;

    /** 유효 사거리 최대 */
    UPROPERTY(EditAnywhere, Category = "Variable|Attack")
    float MaxRange = 1100.f;

    /** 발사할 투사체 클래스 */
    UPROPERTY(EditAnywhere, Category = "Variable|Attack")
    TSubclassOf<APMProjectile> ProjectileClass;

    /** 조준 중 전신 점등 색 -> 추후 조정필요 */
    UPROPERTY(EditAnywhere, Category = "Variable|Attack")
    FLinearColor TelegraphColor = FLinearColor::Red;

    /** 평상시 몸 색 */
    UPROPERTY(EditAnywhere, Category = "Variable|Attack")
    FLinearColor NormalColor = FLinearColor::White;

    /** 발사 직전 방향 고정 시간 */
    UPROPERTY(EditAnywhere, Category = "Variable|Attack")
    float AimLockTime = 0.2f;

    // Runtime State
private:
    /** 태스크 시작 후 경과 시간 */
    float ElapsedTime = 0.f;

    /** 이미 발사했는지 */
    bool bFired = false;

    /** 현재 슬롯 보유 여부 */
    bool bHasSlot = false;

    /** 방향이 고정됐는지 여부 */
    bool bDirectionLocked = false;

    /** 고정된 발사 방향 */
    FVector LockedDirection = FVector::ForwardVector;
};