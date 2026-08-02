#include "PMBTService_DetectPlayer.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Define/PMBlackboardKeys.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

UPMBTService_DetectPlayer::UPMBTService_DetectPlayer()
{
    NodeName = TEXT("Detect Player");

    Interval = 0.2f;
    RandomDeviation = 0.f;

    bCreateNodeInstance = true;
}

void UPMBTService_DetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (AIController == nullptr || BlackboardComp == nullptr)
    {
        return;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(OwnerComp.GetWorld(), 0);
    if (ControlledPawn == nullptr || PlayerPawn == nullptr)
    {
        return;
    }

    const float WorldTime = OwnerComp.GetWorld()->GetTimeSeconds();

    // 타켓 없으먼 대기 상태 -> 인지 거리 체크
    if (BlackboardComp->GetValueAsObject(PMBlackboardKeys::TargetActor) == nullptr)
    {
        const float DistToPlayer = FVector::Dist(ControlledPawn->GetActorLocation(), PlayerPawn->GetActorLocation());
        if (DistToPlayer <= DetectRadius)
        {
            BlackboardComp->SetValueAsObject(PMBlackboardKeys::TargetActor, PlayerPawn);
            BlackboardComp->SetValueAsVector(PMBlackboardKeys::LastKnownLocation, PlayerPawn->GetActorLocation());
            LastSeenTime = WorldTime;
            PathFailStartTime = -1.0f;
        }
        return;
    }

    // 추격 종료 조건 1 : 시작 위치에서 20m 초과 이탈
    const FVector HomeLocation = BlackboardComp->GetValueAsVector(PMBlackboardKeys::HomeLocation);
    if (FVector::Dist(ControlledPawn->GetActorLocation(), HomeLocation) > MaxChaseDistance)
    {
        BlackboardComp->ClearValue(PMBlackboardKeys::TargetActor);
        return;
    }

    // 추격 종료 조건 2 : 경로 탐색 실패 2초 지속
    // 추후 조정필요 : 몬스터마다 0.2초 간격으로 동기 경로탐색 -> 몬스터가 많아지면 프레임 저하, 간격 조절 필요
    UNavigationPath* PathToPlayer = UNavigationSystemV1::FindPathToLocationSynchronously(
        OwnerComp.GetWorld(), ControlledPawn->GetActorLocation(), PlayerPawn->GetActorLocation(), ControlledPawn);
    const bool bHasPath = PathToPlayer != nullptr && PathToPlayer->IsValid() && !PathToPlayer->IsPartial();

    if (bHasPath)
    {
        PathFailStartTime = -1.0f;
    }
    else if (PathFailStartTime < 0.0f)
    {
        PathFailStartTime = WorldTime;
    }
    else if (WorldTime - PathFailStartTime > PathFailTime)
    {
        BlackboardComp->ClearValue(PMBlackboardKeys::TargetActor);
        return;
    }

    // 추격 종료 조건 3 : 시야 상실 3초 지속
    if (AIController->LineOfSightTo(PlayerPawn))
    {
        LastSeenTime = WorldTime;
        BlackboardComp->SetValueAsVector(PMBlackboardKeys::LastKnownLocation, PlayerPawn->GetActorLocation());
    }
    else if (WorldTime - LastSeenTime > LoseSightTime)
    {
        BlackboardComp->ClearValue(PMBlackboardKeys::TargetActor);
    }
}