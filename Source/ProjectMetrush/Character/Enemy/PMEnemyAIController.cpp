#include "PMEnemyAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/PMEnemy.h"
#include "Define/PMBlackboardKeys.h"

void APMEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    APMEnemy* Enemy = Cast<APMEnemy>(InPawn);
    if (Enemy == nullptr || Enemy->GetBehaviorTree() == nullptr)
    {
        return;
    }

    UBehaviorTree* BT = Enemy->GetBehaviorTree();

    // BB 켜고 시작위치 기록 후 BT 실행
    UBlackboardComponent* BlackboardComp = Blackboard;
    if (UseBlackboard(BT->BlackboardAsset, BlackboardComp))
    {
        BlackboardComp->SetValueAsVector(PMBlackboardKeys::HomeLocation, Enemy->GetActorLocation());
        RunBehaviorTree(BT);
    }
}