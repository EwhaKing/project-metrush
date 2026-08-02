#include "PMBTTask_GuardStance.h"

#include "AIController.h"
#include "Kismet/GameplayStatics.h"

UPMBTTask_GuardStance::UPMBTTask_GuardStance()
{
    NodeName = TEXT("Guard Stance");

    bNotifyTick = true;
    bCreateNodeInstance = true;
}

EBTNodeResult::Type UPMBTTask_GuardStance::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
    if (ControlledPawn == nullptr)
    {
        return EBTNodeResult::Failed;
    }

    ElapsedTime = 0.f;
    return EBTNodeResult::InProgress;
}

void UPMBTTask_GuardStance::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(OwnerComp.GetWorld(), 0);
    if (ControlledPawn == nullptr || PlayerPawn == nullptr)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    ElapsedTime += DeltaSeconds;

    // 플레이어를 향해 회전
    const FVector ToPlayer = (PlayerPawn->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal2D();
    if (ToPlayer.IsNearlyZero() == false)
    {
        const FRotator CurrentRot = ControlledPawn->GetActorRotation();
        const FRotator TargetRot(0.f, ToPlayer.Rotation().Yaw, 0.f);
        const FRotator NewRot = FMath::RInterpConstantTo(CurrentRot, TargetRot, DeltaSeconds, RotationSpeed);
        ControlledPawn->SetActorRotation(NewRot);
    }

    // 밀치기 사거리 안이면 종료 -> BT가 ShieldBash 재시도
    const float DistToPlayer = FVector::Dist(ControlledPawn->GetActorLocation(), PlayerPawn->GetActorLocation());
    if (DistToPlayer <= BashRange)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // 재평가 주기 경과 시 종료 -> BT가 보호 위치 다시 계산
    if (ElapsedTime >= RecheckTime)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}