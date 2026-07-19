#include "PMBTTask_RangedAttack.h"

#include "AIController.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Actor/PMProjectile.h"
#include "Character/Enemy/PMEnemy.h"
#include "Subsystem/PMAttackSlotSubsystem.h"

UPMBTTask_RangedAttack::UPMBTTask_RangedAttack()
{
    NodeName = TEXT("Ranged Attack");

    bNotifyTick = true;
    bCreateNodeInstance = true;
}

EBTNodeResult::Type UPMBTTask_RangedAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    APMEnemy* Enemy = AIController ? Cast<APMEnemy>(AIController->GetPawn()) : nullptr;
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(OwnerComp.GetWorld(), 0);
    if (Enemy == nullptr || PlayerPawn == nullptr || ProjectileClass == nullptr)
    {
        return EBTNodeResult::Failed;
    }

    ElapsedTime = 0.f;
    bFired = false;
    bHasSlot = false;
    bDirectionLocked = false;

    // 거리 확인 -> 시야 확인 -> 슬롯 요청 -> 조준
    const float DistToPlayer = FVector::Dist(Enemy->GetActorLocation(), PlayerPawn->GetActorLocation());
    if (DistToPlayer < MinRange || DistToPlayer > MaxRange)
    {
        return EBTNodeResult::Failed;
    }
    if (AIController->LineOfSightTo(PlayerPawn) == false)
    {
        return EBTNodeResult::Failed;
    }

    UPMAttackSlotSubsystem* SlotSubsystem = OwnerComp.GetWorld()->GetSubsystem<UPMAttackSlotSubsystem>();
    if (SlotSubsystem == nullptr || SlotSubsystem->TryAcquireSlot(SlotType, Enemy) == false)
    {
        return EBTNodeResult::Failed;
    }
    bHasSlot = true;

    // 조준 시작 -> 전신 점등
    Enemy->SetBodyColor(TelegraphColor);

    return EBTNodeResult::InProgress;
}

void UPMBTTask_RangedAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    ElapsedTime += DeltaSeconds;

    if (bFired == false)
    {
        AAIController* AIController = OwnerComp.GetAIOwner();
        APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(OwnerComp.GetWorld(), 0);
        if (ControlledPawn == nullptr || PlayerPawn == nullptr)
        {
            ResetTelegraph(OwnerComp);
            ReleaseHeldSlot(OwnerComp);
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            return;
        }

        // 조준 중 시야가 막히면 슬롯 반환 후 취소
        if (AIController->LineOfSightTo(PlayerPawn) == false)
        {
            ResetTelegraph(OwnerComp);
            ReleaseHeldSlot(OwnerComp);
            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            return;
        }

        // 발사 직전 방향 고정
        if (bDirectionLocked == false && ElapsedTime >= AimTime - AimLockTime)
        {
            bDirectionLocked = true;
            LockedDirection =
                (PlayerPawn->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal();
        }

        // 조준선: 고정 전 -> 얇은 선, 고정 후 -> 굵은 선
        const FVector LineStart = ControlledPawn->GetActorLocation();
        const FVector LineEnd = bDirectionLocked
            ? LineStart + LockedDirection * MaxRange
            : PlayerPawn->GetActorLocation();
        const float Thickness = bDirectionLocked ? 5.f : 1.5f;
        DrawDebugLine(OwnerComp.GetWorld(), LineStart, LineEnd, FColor::Orange, false, -1.f, 0, Thickness);

        // 조준 완료 -> 발사
        if (ElapsedTime >= AimTime)
        {
            bFired = true;
            ResetTelegraph(OwnerComp);
            FireProjectile(OwnerComp);
        }
        return;
    }

    // 재장전 1초 경과 -> 슬롯 반환 후 종료
    if (ElapsedTime >= AimTime + ReloadTime)
    {
        ReleaseHeldSlot(OwnerComp);
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}

EBTNodeResult::Type UPMBTTask_RangedAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ResetTelegraph(OwnerComp);
    ReleaseHeldSlot(OwnerComp);

    return Super::AbortTask(OwnerComp, NodeMemory);
}

void UPMBTTask_RangedAttack::FireProjectile(UBehaviorTreeComponent& OwnerComp)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(OwnerComp.GetWorld(), 0);
    if (ControlledPawn == nullptr || PlayerPawn == nullptr)
    {
        return;
    }

    // 고정된 방향으로 발사
    const FVector FireDirection = bDirectionLocked
        ? LockedDirection
        : (PlayerPawn->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal();

    // 캡슐과 겹치지 않게 몸 앞 1m 지점에서 생성
    const FVector SpawnLocation = ControlledPawn->GetActorLocation() + FireDirection * 100.f;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Instigator = ControlledPawn;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    OwnerComp.GetWorld()->SpawnActor<APMProjectile>(
        ProjectileClass, SpawnLocation, FireDirection.Rotation(), SpawnParams);
}

void UPMBTTask_RangedAttack::ResetTelegraph(UBehaviorTreeComponent& OwnerComp)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    APMEnemy* Enemy = AIController ? Cast<APMEnemy>(AIController->GetPawn()) : nullptr;
    if (Enemy != nullptr)
    {
        Enemy->SetBodyColor(NormalColor);
    }
}

void UPMBTTask_RangedAttack::ReleaseHeldSlot(UBehaviorTreeComponent& OwnerComp)
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