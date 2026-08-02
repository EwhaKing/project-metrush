#include "PMBTTask_MeleeAttack.h"

#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Enemy/PMEnemy.h"
#include "Subsystem/PMAttackSlotSubsystem.h"

UPMBTTask_MeleeAttack::UPMBTTask_MeleeAttack()
{
    NodeName = TEXT("Melee Attack");

    bNotifyTick = true;
    bCreateNodeInstance = true;
}

EBTNodeResult::Type UPMBTTask_MeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(OwnerComp.GetWorld(), 0);
    if (PlayerPawn == nullptr || FVector::Dist(Enemy->GetActorLocation(), PlayerPawn->GetActorLocation()) > HitRange)
    {
        return EBTNodeResult::Failed;
    }

    // 공격 슬롯 요청
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

void UPMBTTask_MeleeAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    ElapsedTime += DeltaSeconds;

    // 준비 0.5초 종료 시점: 점등을 끄고 공격 판정 1회
    // -> 추후 조정필요 판정 유지시간은 애니메이션 도입 후 설정
    if (!bHitChecked && ElapsedTime >= PrepareTime)
    {
        bHitChecked = true;
        ResetTelegraph(OwnerComp);
        DoHitCheck(OwnerComp);
    }

    // 판정 후 후딜레이 0.7초까지 경과하면 태스크 종료
    if (bHitChecked && ElapsedTime >= PrepareTime + AfterDelayTime)
    {
        ReleaseHeldSlot(OwnerComp);
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}

EBTNodeResult::Type UPMBTTask_MeleeAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ReleaseHeldSlot(OwnerComp);
    // 공격 도중 중단 시 초기화
    ResetTelegraph(OwnerComp);

    return Super::AbortTask(OwnerComp, NodeMemory);
}

void UPMBTTask_MeleeAttack::DoHitCheck(UBehaviorTreeComponent& OwnerComp)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(OwnerComp.GetWorld(), 0);
    if (ControlledPawn == nullptr || PlayerPawn == nullptr)
    {
        return;
    }

    // 추후 조정필요 : 전방 각도 제한 여부
    const float DistToPlayer = FVector::Dist(ControlledPawn->GetActorLocation(), PlayerPawn->GetActorLocation());
    if (DistToPlayer > HitRange)
    {
        // 추후 조정필요 : 준비 시간 동안 플레이어가 회피/이탈 -> 헛스윙
        UE_LOG(LogTemp, Log, TEXT("Player Dodged"));
        return;
    }

    // 피해 전달. 무적/저스트 판단은 플레이어 TakeDamage가 일괄 처리
    UGameplayStatics::ApplyDamage(PlayerPawn, Damage,
        AIController, ControlledPawn, nullptr);
}

void UPMBTTask_MeleeAttack::ResetTelegraph(UBehaviorTreeComponent& OwnerComp)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    APMEnemy* Enemy = AIController ? Cast<APMEnemy>(AIController->GetPawn()) : nullptr;
    if (Enemy != nullptr)
    {
        Enemy->SetBodyColor(NormalColor);
    }
}

void UPMBTTask_MeleeAttack::ReleaseHeldSlot(UBehaviorTreeComponent& OwnerComp)
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