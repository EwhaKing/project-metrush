#include "PMBTTask_FindFirePosition.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Define/PMBlackboardKeys.h"

UPMBTTask_FindFirePosition::UPMBTTask_FindFirePosition()
{
    NodeName = TEXT("Find Fire Position");
}

EBTNodeResult::Type UPMBTTask_FindFirePosition::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    const FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector DirFromPlayer = (ControlledPawn->GetActorLocation() - PlayerLocation).GetSafeNormal2D();
    if (DirFromPlayer.IsNearlyZero())
    {
        DirFromPlayer = FVector::ForwardVector;
    }

    // 현재 방향을 우선 시도, 시야 막히면 좌우로 틀며 플레이어를 찾음
    // 추후 조정필요 : 후보 각도 개수/간격, 넓히면 자리를 잘 찾지만 매 탐색 비용이 늘어남
    const float CandidateAngles[] = { 0.f, 35.f, -35.f, 70.f, -70.f };

    FVector FallbackLocation = FVector::ZeroVector;
    bool bHasFallback = false;

    for (const float Angle : CandidateAngles)
    {
        const FVector Candidate =
            PlayerLocation + DirFromPlayer.RotateAngleAxis(Angle, FVector::UpVector) * FireDistance;

        FNavLocation NavCandidate;
        if (NavSys->GetRandomPointInNavigableRadius(Candidate, 150.f, NavCandidate) == false)
        {
            continue;
        }

        // 후보 지점 -> 플레이어 시야 확인
        FHitResult HitResult;
        FCollisionQueryParams TraceParams;
        TraceParams.AddIgnoredActor(ControlledPawn);
        const FVector TraceStart = NavCandidate.Location + FVector(0.f, 0.f, 50.f);
        const bool bHit = OwnerComp.GetWorld()->LineTraceSingleByChannel(
            HitResult, TraceStart, PlayerLocation, ECC_Visibility, TraceParams);

        // 아무것도 안 막혔거나 처음 맞은 게 플레이어면 시야 확보
        if (bHit == false || HitResult.GetActor() == PlayerPawn)
        {
            BlackboardComp->SetValueAsVector(PMBlackboardKeys::FirePosition, NavCandidate.Location);
            return EBTNodeResult::Succeeded;
        }

        // 시야는 없지만 이동은 가능한 후보를 예비로 저장
        if (bHasFallback == false)
        {
            FallbackLocation = NavCandidate.Location;
            bHasFallback = true;
        }
    }

    // 시야 확보 지점이 없으면 예비 지점으로 이동 (도착 후 다음 루프에서 재탐색)
    if (bHasFallback)
    {
        BlackboardComp->SetValueAsVector(PMBlackboardKeys::FirePosition, FallbackLocation);
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}