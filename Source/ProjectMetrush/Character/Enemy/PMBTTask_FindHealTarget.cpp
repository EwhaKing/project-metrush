#include "PMBTTask_FindHealTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Enemy/PMEnemy.h"
#include "Define/PMBlackboardKeys.h"

UPMBTTask_FindHealTarget::UPMBTTask_FindHealTarget()
{
    NodeName = TEXT("Find Heal Target");
}

EBTNodeResult::Type UPMBTTask_FindHealTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    APMEnemy* Healer = AIController ? Cast<APMEnemy>(AIController->GetPawn()) : nullptr;
    if (Healer == nullptr || BlackboardComp == nullptr)
    {
        return EBTNodeResult::Failed;
    }

    // 월드의 모든 몬스터 수집
    TArray<AActor*> AllEnemies;
    UGameplayStatics::GetAllActorsOfClass(OwnerComp.GetWorld(), APMEnemy::StaticClass(), AllEnemies);

    const FVector HealerLocation = Healer->GetActorLocation();
    const float HealRangeSq = HealRange * HealRange;

    APMEnemy* BestTarget = nullptr;
    float BestRatio = 1.f;
    int32 BestPriority = MAX_int32;

    for (AActor* Actor : AllEnemies)
    {
        APMEnemy* Ally = Cast<APMEnemy>(Actor);

        // 자신은 회복하지 않음, 죽은 아군 제외
        if (Ally == nullptr || Ally == Healer || Ally->IsDead())
        {
            continue;
        }

        // 부상당한 아군만
        const float HPRatio = Ally->GetHPRatio();
        if (HPRatio >= 1.f)
        {
            continue;
        }

        // 회복 범위 확인
        if (FVector::DistSquared(HealerLocation, Ally->GetActorLocation()) > HealRangeSq)
        {
            continue;
        }

        // 대상과 시야 확인
        if (AIController->LineOfSightTo(Ally) == false)
        {
            continue;
        }

        // HP 비율이 더 낮으면 교체, 동률이면 방패 > 근접 > 원거리
        const int32 TypePriority = GetTypePriority(Ally->GetEnemyType());
        const bool bLowerHP = HPRatio < BestRatio - KINDA_SMALL_NUMBER;
        const bool bSameHPButHigherPriority =
            FMath::IsNearlyEqual(HPRatio, BestRatio) && TypePriority < BestPriority;

        if (BestTarget == nullptr || bLowerHP || bSameHPButHigherPriority)
        {
            BestTarget = Ally;
            BestRatio = HPRatio;
            BestPriority = TypePriority;
        }
    }

    // 회복할 아군이 없으면 실패 -> BT가 후방 위치 유지 분기로 넘어감
    if (BestTarget == nullptr)
    {
        BlackboardComp->ClearValue(PMBlackboardKeys::HealTarget);
        return EBTNodeResult::Failed;
    }

    BlackboardComp->SetValueAsObject(PMBlackboardKeys::HealTarget, BestTarget);
    return EBTNodeResult::Succeeded;
}

int32 UPMBTTask_FindHealTarget::GetTypePriority(EPMEnemyType TargetType) const
{
    // 우선순위 : 방패형 > 근접형 > 원거리형
    switch (TargetType)
    {
    case EPMEnemyType::Shield:
        return 0;
    case EPMEnemyType::Melee:
        return 1;
    case EPMEnemyType::Ranged:
        return 2;
    default:
        return 3;
    }
}