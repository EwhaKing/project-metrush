#include "PMAttackSlotSubsystem.h"

int32 UPMAttackSlotSubsystem::GetCapacity(EPMAttackSlotType SlotType) const
{
    // 몬스터 수
    // 추후 조정필요 : 동시에 공격할 수 있는 몬스터 수, 난이도에 직결되는 값
    switch (SlotType)
    {
    case EPMAttackSlotType::Melee:      return 3;
    case EPMAttackSlotType::Ranged:     return 2;
    case EPMAttackSlotType::ShieldBash: return 1;
    case EPMAttackSlotType::Heal:       return 1;
    }
    return 0;
}

void UPMAttackSlotSubsystem::CleanUp(EPMAttackSlotType SlotType)
{
    const float WorldTime = GetWorld()->GetTimeSeconds();

    // 사망한 슬롯 보유 몬스터 제거
    Holders.FindOrAdd(SlotType).RemoveAll([](const TWeakObjectPtr<AActor>& Holder)
    {
        return Holder.IsValid() == false;
    });

    // 사망했거나 3초 넘게 재요청이 없는 대기 몬스터 제거
    // 추후 조정필요 : 대기열 만료 시간 3초가 하드코딩, 변수로 뺄지 결정 필요
    Waiters.FindOrAdd(SlotType).RemoveAll([WorldTime](const FPMSlotWaiter& Waiter)
    {
        return Waiter.Actor.IsValid() == false || WorldTime - Waiter.LastAskTime > 3.0f;
    });
}

bool UPMAttackSlotSubsystem::TryAcquireSlot(EPMAttackSlotType SlotType, AActor* Requester)
{
    if (Requester == nullptr)
    {
        return false;
    }

    CleanUp(SlotType);

    TArray<TWeakObjectPtr<AActor>>& TypeHolders = Holders.FindOrAdd(SlotType);
    TArray<FPMSlotWaiter>& TypeWaiters = Waiters.FindOrAdd(SlotType);

    // 슬롯 중복 획득 방지
    if (TypeHolders.Contains(Requester))
    {
        return true;
    }

    const float WorldTime = GetWorld()->GetTimeSeconds();

    // 대기열에 없을 시 신규 등록
    int32 MyIndex = TypeWaiters.IndexOfByPredicate(
        [Requester](const FPMSlotWaiter& Waiter) { return Waiter.Actor == Requester; });
    if (MyIndex == INDEX_NONE)
    {
        FPMSlotWaiter NewWaiter;
        NewWaiter.Actor = Requester;
        NewWaiter.FirstAskTime = WorldTime;
        NewWaiter.LastAskTime = WorldTime;
        MyIndex = TypeWaiters.Add(NewWaiter);
    }
    else
    {
        TypeWaiters[MyIndex].LastAskTime = WorldTime;
    }

    // 빈 슬롯이 없으면 실패 (대기열에는 남아 있음)
    if (TypeHolders.Num() >= GetCapacity(SlotType))
    {
        return false;
    }

    // 빈 슬롯이 있어도 나보다 먼저 기다린 몬스터가 있으면 대기
    for (int32 i = 0; i < TypeWaiters.Num(); ++i)
    {
        if (i != MyIndex && TypeWaiters[i].FirstAskTime < TypeWaiters[MyIndex].FirstAskTime)
        {
            return false;
        }
    }

    // 획득: 대기열에서 빼고 보유자로 등록
    TypeWaiters.RemoveAt(MyIndex);
    TypeHolders.Add(Requester);
    LogSlotStatus(SlotType);
    return true;
}

void UPMAttackSlotSubsystem::ReleaseSlot(EPMAttackSlotType SlotType, AActor* Requester)
{
    if (Requester == nullptr)
    {
        return;
    }

    Holders.FindOrAdd(SlotType).Remove(Requester);
    LogSlotStatus(SlotType);
}

void UPMAttackSlotSubsystem::LogSlotStatus(EPMAttackSlotType SlotType)
{
    const TCHAR* SlotName = TEXT("");
    switch (SlotType)
    {
    case EPMAttackSlotType::Melee:      SlotName = TEXT("Melee");  break;
    case EPMAttackSlotType::Ranged:     SlotName = TEXT("Ranged"); break;
    case EPMAttackSlotType::ShieldBash: SlotName = TEXT("Shield"); break;
    case EPMAttackSlotType::Heal:       SlotName = TEXT("Heal");   break;
    }

    UE_LOG(LogTemp, Log, TEXT("%s Slot: %d / %d"), SlotName,
        Holders.FindOrAdd(SlotType).Num(), GetCapacity(SlotType));
}

bool UPMAttackSlotSubsystem::IsHolding(EPMAttackSlotType SlotType, AActor* Actor) const
{
    const TArray<TWeakObjectPtr<AActor>>* TypeHolders = Holders.Find(SlotType);
    return TypeHolders != nullptr && TypeHolders->Contains(Actor);
}