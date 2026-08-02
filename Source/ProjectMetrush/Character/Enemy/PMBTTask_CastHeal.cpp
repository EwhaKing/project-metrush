
#include "PMBTTask_CastHeal.h"

#include "AIController.h"
#include "DrawDebugHelpers.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/PMEnemy.h"
#include "Define/PMBlackboardKeys.h"
#include "Subsystem/PMAttackSlotSubsystem.h"

UPMBTTask_CastHeal::UPMBTTask_CastHeal()
{
    NodeName = TEXT("Cast Heal");

    bNotifyTick = true;
    bCreateNodeInstance = true;
}

EBTNodeResult::Type UPMBTTask_CastHeal::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    APMEnemy* Healer = AIController ? Cast<APMEnemy>(AIController->GetPawn()) : nullptr;
    APMEnemy* Target = GetHealTarget(OwnerComp);
    if (Healer == nullptr || Target == nullptr)
    {
        return EBTNodeResult::Failed;
    }

    ElapsedTime = 0.f;
    bHasSlot = false;

    // 회복 조건 : 생존, 범위, 시야
    const bool bCanHeal =
        Target->IsDead() == false &&
        FVector::Dist(Healer->GetActorLocation(), Target->GetActorLocation()) <= HealRange &&
        AIController->LineOfSightTo(Target);
    if (bCanHeal == false)
    {
        return EBTNodeResult::Failed;
    }

    // 회복 슬롯 요청
    UPMAttackSlotSubsystem* SlotSubsystem = OwnerComp.GetWorld()->GetSubsystem<UPMAttackSlotSubsystem>();
    if (SlotSubsystem == nullptr || SlotSubsystem->TryAcquireSlot(SlotType, Healer) == false)
    {
        return EBTNodeResult::Failed;
    }
    bHasSlot = true;

    // 시전 중 피격 판정 기준값
    StartDamagedCount = Healer->GetDamagedCount();

    // 시전 시작 : 전신 점등
    Healer->SetBodyColor(TelegraphColor);

    return EBTNodeResult::InProgress;
}

void UPMBTTask_CastHeal::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    ElapsedTime += DeltaSeconds;

    AAIController* AIController = OwnerComp.GetAIOwner();
    APMEnemy* Healer = AIController ? Cast<APMEnemy>(AIController->GetPawn()) : nullptr;
    APMEnemy* Target = GetHealTarget(OwnerComp);
    if (Healer == nullptr || Target == nullptr)
    {
        CancelCast(OwnerComp, TEXT("Invalid Target"));
        return;
    }

    // 취소 조건 : 힐러 피격
    if (Healer->GetDamagedCount() != StartDamagedCount)
    {
        CancelCast(OwnerComp, TEXT("Healer Damaged"));
        return;
    }

    // 취소 조건 : 대상 사망
    if (Target->IsDead())
    {
        CancelCast(OwnerComp, TEXT("Target Died"));
        return;
    }

    // 취소 조건 : 대상이 회복 범위 밖으로 이동
    if (FVector::Dist(Healer->GetActorLocation(), Target->GetActorLocation()) > HealRange)
    {
        CancelCast(OwnerComp, TEXT("Target Out Of Range"));
        return;
    }

    // 취소 조건 : 대상과 시야 차단
    if (AIController->LineOfSightTo(Target) == false)
    {
        CancelCast(OwnerComp, TEXT("Lost Sight"));
        return;
    }

    DrawHealLink(OwnerComp, Healer, Target);

    // 시전 완료 -> 회복 적용 후 슬롯 반환
    if (ElapsedTime >= CastTime)
    {
        ResetTelegraph(OwnerComp);
        ApplyHeal(OwnerComp);
        ReleaseHeldSlot(OwnerComp);
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}

EBTNodeResult::Type UPMBTTask_CastHeal::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 경직, 사망, 상위 분기 전환으로 중단될 때
    ResetTelegraph(OwnerComp);
    ReleaseHeldSlot(OwnerComp);

    return Super::AbortTask(OwnerComp, NodeMemory);
}

void UPMBTTask_CastHeal::ApplyHeal(UBehaviorTreeComponent& OwnerComp)
{
    APMEnemy* Target = GetHealTarget(OwnerComp);
    if (Target == nullptr || Target->IsDead())
    {
        return;
    }

    // 회복량 = 대상 최대 HP의 20%
    Target->Heal(Target->GetMaxHP() * HealPercent);

    // 추후 조정필요 : 회복 완료 이펙트
}

void UPMBTTask_CastHeal::DrawHealLink(UBehaviorTreeComponent& OwnerComp, APMEnemy* Healer, APMEnemy* Target) const
{
    // 시전이 진행될수록 연결선이 밝아짐
    const float Progress = FMath::Clamp(ElapsedTime / CastTime, 0.f, 1.f);
    const uint8 Brightness = static_cast<uint8>(FMath::Lerp(60.f, 255.f, Progress));
    const FColor LinkColor(0, Brightness, 0);

    DrawDebugLine(OwnerComp.GetWorld(), Healer->GetActorLocation(), Target->GetActorLocation(),
        LinkColor, false, -1.f, 0, 4.f);

    // 회복 대상 표시
    DrawDebugSphere(OwnerComp.GetWorld(), Target->GetActorLocation() + FVector(0.f, 0.f, 120.f),
        30.f, 12, LinkColor, false, -1.f, 0, 2.f);

    // 추후 조정필요 : 디버그 표시를 실제 이펙트로 교체
}

void UPMBTTask_CastHeal::CancelCast(UBehaviorTreeComponent& OwnerComp, const TCHAR* Reason)
{
    UE_LOG(LogTemp, Log, TEXT("Heal Canceled : %s"), Reason);

    // 취소 시 회복은 적용하지 않고 슬롯만 반환
    ResetTelegraph(OwnerComp);
    ReleaseHeldSlot(OwnerComp);
    FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
}

void UPMBTTask_CastHeal::ResetTelegraph(UBehaviorTreeComponent& OwnerComp)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    APMEnemy* Healer = AIController ? Cast<APMEnemy>(AIController->GetPawn()) : nullptr;
    if (Healer != nullptr)
    {
        Healer->SetBodyColor(NormalColor);
    }
}

void UPMBTTask_CastHeal::ReleaseHeldSlot(UBehaviorTreeComponent& OwnerComp)
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

APMEnemy* UPMBTTask_CastHeal::GetHealTarget(UBehaviorTreeComponent& OwnerComp) const
{
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (BlackboardComp == nullptr)
    {
        return nullptr;
    }

    return Cast<APMEnemy>(BlackboardComp->GetValueAsObject(PMBlackboardKeys::HealTarget));
}