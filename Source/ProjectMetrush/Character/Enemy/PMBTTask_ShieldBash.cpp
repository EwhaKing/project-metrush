#include "PMBTTask_ShieldBash.h"

#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Enemy/PMEnemy.h"
#include "Subsystem/PMAttackSlotSubsystem.h"

UPMBTTask_ShieldBash::UPMBTTask_ShieldBash()
{
    NodeName = TEXT("Shield Bash");

    bNotifyTick = true;
    bCreateNodeInstance = true;
}

EBTNodeResult::Type UPMBTTask_ShieldBash::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    APMEnemy* Enemy = AIController ? Cast<APMEnemy>(AIController->GetPawn()) : nullptr;
    if (Enemy == nullptr)
    {
        return EBTNodeResult::Failed;
    }

    // 스폰 직후에는 공격하지 않음
    if (Enemy->CanAttack() == false)
    {
        return EBTNodeResult::Failed;
    }

    ElapsedTime = 0.f;
    bHitChecked = false;
    bHasSlot = false;

    // 밀치기 사거리 밖이면 시작하지 않음 (BT가 정면 방어/이동 분기로)
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(OwnerComp.GetWorld(), 0);
    if (PlayerPawn == nullptr || FVector::Dist(Enemy->GetActorLocation(), PlayerPawn->GetActorLocation()) > HitRange)
    {
        return EBTNodeResult::Failed;
    }

    // 밀치기 슬롯 요청
    UPMAttackSlotSubsystem* SlotSubsystem = OwnerComp.GetWorld()->GetSubsystem<UPMAttackSlotSubsystem>();
    if (SlotSubsystem == nullptr || SlotSubsystem->TryAcquireSlot(SlotType, Enemy) == false)
    {
        return EBTNodeResult::Failed;
    }
    bHasSlot = true;

    // 준비 단계 시작 : 전신 점등 텔레그래프
    Enemy->SetBodyColor(TelegraphColor);

    return EBTNodeResult::InProgress;
}

void UPMBTTask_ShieldBash::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    ElapsedTime += DeltaSeconds;

    // 준비 종료 시점 : 점등을 끄고 밀치기 판정 1회
    // -> 추후 조정필요 : 판정 유지시간 조절
    if (!bHitChecked && ElapsedTime >= PrepareTime)
    {
        bHitChecked = true;
        ResetTelegraph(OwnerComp);
        DoBash(OwnerComp);
    }

    // 판정 후 후딜레이 1초까지 경과하면 태스크 종료
    if (bHitChecked && ElapsedTime >= PrepareTime + AfterDelayTime)
    {
        ReleaseHeldSlot(OwnerComp);
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}

EBTNodeResult::Type UPMBTTask_ShieldBash::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ReleaseHeldSlot(OwnerComp);
    // 밀치기 도중 중단 시 초기화
    ResetTelegraph(OwnerComp);

    return Super::AbortTask(OwnerComp, NodeMemory);
}

void UPMBTTask_ShieldBash::DoBash(UBehaviorTreeComponent& OwnerComp)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(OwnerComp.GetWorld(), 0);
    if (ControlledPawn == nullptr || PlayerPawn == nullptr)
    {
        return;
    }

    const float DistToPlayer = FVector::Dist(ControlledPawn->GetActorLocation(), PlayerPawn->GetActorLocation());
    if (DistToPlayer > HitRange)
    {
        // 준비 시간 동안 플레이어가 회피/이탈 -> 헛방
        UE_LOG(LogTemp, Log, TEXT("Player Dodged Bash"));
        return;
    }

    // 넉백 방향 : 방패형 -> 플레이어
    const FVector KnockDir = (PlayerPawn->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal2D();

    // 추후 조정필요 : 플레이어 낮은 피해 + 짧은 넉백 처리 / 밀치기 피해량, 넉백 거리 수치 조절
    UE_LOG(LogTemp, Log, TEXT("Shield Bash Hit"));
}

void UPMBTTask_ShieldBash::ResetTelegraph(UBehaviorTreeComponent& OwnerComp)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    APMEnemy* Enemy = AIController ? Cast<APMEnemy>(AIController->GetPawn()) : nullptr;
    if (Enemy != nullptr)
    {
        Enemy->SetBodyColor(NormalColor);
    }
}

void UPMBTTask_ShieldBash::ReleaseHeldSlot(UBehaviorTreeComponent& OwnerComp)
{
    if (bHasSlot == false)
    {
        return;
    }
    bHasSlot = false;

    AAIController* AIController = OwnerComp.GetAIOwner();
    AActor* OwnerActor = AIController ? AIController->GetPawn() : nullptr;
    UPMAttackSlotSubsystem* SlotSubsystem = OwnerComp.GetWorld()->GetSubsystem<UPMAttackSlotSubsystem>();
    if (OwnerActor != nullptr && SlotSubsystem != nullptr)
    {
        SlotSubsystem->ReleaseSlot(SlotType, OwnerActor);
    }
}