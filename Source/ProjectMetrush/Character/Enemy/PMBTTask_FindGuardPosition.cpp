#include "PMBTTask_FindGuardPosition.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Enemy/PMEnemy.h"
#include "Define/PMBlackboardKeys.h"

UPMBTTask_FindGuardPosition::UPMBTTask_FindGuardPosition()
{
    NodeName = TEXT("Find Guard Position");
}

EBTNodeResult::Type UPMBTTask_FindGuardPosition::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(OwnerComp.GetWorld(), 0);
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(OwnerComp.GetWorld());
    if (ControlledPawn == nullptr || PlayerPawn == nullptr || BlackboardComp == nullptr || NavSys == nullptr)
    {
        return EBTNodeResult::Failed;
    }

    const FVector MyLocation = ControlledPawn->GetActorLocation();

    // 월드의 모든 몬스터 수집
    TArray<AActor*> AllEnemies;
    UGameplayStatics::GetAllActorsOfClass(OwnerComp.GetWorld(), APMEnemy::StaticClass(), AllEnemies);

    // 우선순위 : 힐러형 -> 원거리형. 같은 종류 안에서는 방패형에게 가장 가까운 대상
    const EPMEnemyType PriorityTypes[] = { EPMEnemyType::Healer, EPMEnemyType::Ranged };
    const float SearchRadiusSq = SearchRadius * SearchRadius;

    APMEnemy* Protectee = nullptr;
    for (const EPMEnemyType WantType : PriorityTypes)
    {
        float BestDistSq = SearchRadiusSq;
        for (AActor* Actor : AllEnemies)
        {
            APMEnemy* Enemy = Cast<APMEnemy>(Actor);
            if (Enemy == nullptr || Enemy == ControlledPawn || Enemy->IsDead())
            {
                continue;
            }
            if (Enemy->GetEnemyType() != WantType)
            {
                continue;
            }

            const float DistSq = FVector::DistSquared2D(MyLocation, Enemy->GetActorLocation());
            if (DistSq <= BestDistSq)
            {
                BestDistSq = DistSq;
                Protectee = Enemy;
            }
        }

        // 힐러에서 찾으면 원거리는 보지 않음
        if (Protectee != nullptr)
        {
            break;
        }
    }

    // 보호 대상이 없으면 실패 -> BT가 플레이어 접근 분기로 넘어감
    if (Protectee == nullptr)
    {
        return EBTNodeResult::Failed;
    }

    // 플레이어와 보호 대상 사이, 플레이어 쪽으로 GuardStandoff 만큼
    const FVector ProtLocation = Protectee->GetActorLocation();
    const FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector ToPlayer = (PlayerLocation - ProtLocation).GetSafeNormal2D();
    if (ToPlayer.IsNearlyZero())
    {
        ToPlayer = ControlledPawn->GetActorForwardVector().GetSafeNormal2D();
    }

    // 대상 - 플레이어 중간을 넘지 않도록 제한
    const float DistToPlayer = FVector::Dist2D(ProtLocation, PlayerLocation);
    const float Standoff = FMath::Min(GuardStandoff, DistToPlayer * 0.5f);
    const FVector DesiredPos = ProtLocation + ToPlayer * Standoff;

    // 이동 가능한 위치로 투영
    FNavLocation NavPos;
    if (NavSys->ProjectPointToNavigation(DesiredPos, NavPos, FVector(200.f, 200.f, 500.f)) == false)
    {
        return EBTNodeResult::Failed;
    }

    BlackboardComp->SetValueAsVector(PMBlackboardKeys::GuardPosition, NavPos.Location);

    return EBTNodeResult::Succeeded;
}