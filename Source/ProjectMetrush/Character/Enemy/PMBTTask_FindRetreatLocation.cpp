#include "PMBTTask_FindRetreatLocation.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Define/PMBlackboardKeys.h"

UPMBTTask_FindRetreatLocation::UPMBTTask_FindRetreatLocation()
{
    NodeName = TEXT("Find Retreat Location");
}

EBTNodeResult::Type UPMBTTask_FindRetreatLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    // 플레이어가 3m 밖이면 후퇴 x -> 다음 분기(공격)
    const FVector PawnLocation = ControlledPawn->GetActorLocation();
    const FVector PlayerLocation = PlayerPawn->GetActorLocation();
    const float DistToPlayer = FVector::Dist(PawnLocation, PlayerLocation);
    if (DistToPlayer > RetreatTriggerRange)
    {
        return EBTNodeResult::Failed;
    }

    // 플레이어 반대 방향으로 후퇴
    FVector AwayDir = (PawnLocation - PlayerLocation).GetSafeNormal2D();
    if (AwayDir.IsNearlyZero())
    {
        AwayDir = ControlledPawn->GetActorForwardVector() * -1.f;
    }

    const FVector Candidate = PawnLocation + AwayDir * RetreatDistance;

    FNavLocation NavCandidate;
    if (NavSys->GetRandomPointInNavigableRadius(Candidate, 100.f, NavCandidate) == false)
    {
        // 뒤가 벽/낭떠러지면 후퇴 실패
        return EBTNodeResult::Failed;
    }

    // 찾은 점이 지금보다 플레이어에게 가까우면 후퇴 실패
    if (FVector::Dist(NavCandidate.Location, PlayerLocation) <= DistToPlayer)
    {
        return EBTNodeResult::Failed;
    }

    BlackboardComp->SetValueAsVector(PMBlackboardKeys::RetreatLocation, NavCandidate.Location);
    return EBTNodeResult::Succeeded;
}