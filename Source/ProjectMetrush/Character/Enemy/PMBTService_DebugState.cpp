#include "PMBTService_DebugState.h"

#include "AIController.h"
#include "DrawDebugHelpers.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/PMEnemy.h"
#include "Define/PMBlackboardKeys.h"

static TAutoConsoleVariable<int32> CVarShowEnemyDebug(
    TEXT("PM.ShowEnemyDebug"),
    0,
    TEXT("몬스터 상태와 범위 디버그 표시 (0: 끄기, 1: 켜기)"),
    ECVF_Cheat);

UPMBTService_DebugState::UPMBTService_DebugState()
{
    NodeName = TEXT("Debug State");

    Interval = 0.1f;
    RandomDeviation = 0.f;
}

void UPMBTService_DebugState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    if (CVarShowEnemyDebug.GetValueOnGameThread() == 0)
    {
        return;
    }

    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    APMEnemy* Enemy = AIController ? Cast<APMEnemy>(AIController->GetPawn()) : nullptr;
    if (Enemy == nullptr || BlackboardComp == nullptr)
    {
        return;
    }

    // 현재 상태 판별 (우선순위가 높은 것부터)
    FString StateText;
    if (Enemy->CanAttack() == false)
    {
        StateText = TEXT("SpawnDelay");
    }
    else if (BlackboardComp->GetValueAsBool(PMBlackboardKeys::IsStaggered))
    {
        StateText = TEXT("Staggered");
    }
    else if (BlackboardComp->GetValueAsObject(PMBlackboardKeys::TargetActor) != nullptr)
    {
        StateText = TEXT("Combat");
    }
    else
    {
        StateText = TEXT("Idle");
    }

    // 몬스터 머리 위에 상태와 HP 표시
    const FString DebugText = FString::Printf(TEXT("%s / HP %.0f%%"), *StateText, Enemy->GetHPRatio() * 100.f);
    DrawDebugString(OwnerComp.GetWorld(), FVector(0.f, 0.f, 140.f), DebugText, Enemy, FColor::White, Interval);

    // 인지 범위(노랑), 공격 범위(빨강)
    const FVector EnemyLocation = Enemy->GetActorLocation();
    DrawDebugCircle(OwnerComp.GetWorld(), EnemyLocation, DetectRadius, 32, FColor::Yellow,
        false, Interval, 0, 2.f, FVector(1.f, 0.f, 0.f), FVector(0.f, 1.f, 0.f), false);
    DrawDebugCircle(OwnerComp.GetWorld(), EnemyLocation, AttackRange, 24, FColor::Red,
        false, Interval, 0, 2.f, FVector(1.f, 0.f, 0.f), FVector(0.f, 1.f, 0.f), false);
}