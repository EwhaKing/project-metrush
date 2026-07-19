#include "PMBTTask_FindWaitLocation.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Define/PMBlackboardKeys.h"

UPMBTTask_FindWaitLocation::UPMBTTask_FindWaitLocation()
{
    NodeName = TEXT("Find Wait Location");
}

EBTNodeResult::Type UPMBTTask_FindWaitLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(OwnerComp.GetWorld(), 0);
    if (ControlledPawn == nullptr || PlayerPawn == nullptr || BlackboardComp == nullptr)
    {
        return EBTNodeResult::Failed;
    }

    // 슬롯 받지 못한 경우 측면으로 이동
    const FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector DirFromPlayer = (ControlledPawn->GetActorLocation() - PlayerLocation).GetSafeNormal2D();
    if (DirFromPlayer.IsNearlyZero())
    {
        DirFromPlayer = FVector::ForwardVector;
    }

    const float SideAngle = FMath::FRandRange(-MaxSideAngle, MaxSideAngle);
    const FVector Candidate = PlayerLocation + DirFromPlayer.RotateAngleAxis(SideAngle, FVector::UpVector) * WaitRadius;

    // navmesh에서 빈 위치 찾기
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(OwnerComp.GetWorld());
    FNavLocation WaitLocation;
    if (NavSys == nullptr || NavSys->GetRandomPointInNavigableRadius(Candidate, 150.f, WaitLocation) == false)
    {
        return EBTNodeResult::Failed;
    }

    BlackboardComp->SetValueAsVector(PMBlackboardKeys::WaitLocation, WaitLocation.Location);
    return EBTNodeResult::Succeeded;
}