#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Define/PMAttackSlotType.h"
#include "PMAttackSlotSubsystem.generated.h"

/** 대기 중인 몬스터 정보 */
struct FPMSlotWaiter
{
    /** 대기 중인 몬스터 */
    TWeakObjectPtr<AActor> Actor;

    /** 처음 요청에 실패한 시간 */
    float FirstAskTime = 0.f;

    /** 마지막으로 요청한 시간 */
    float LastAskTime = 0.f;
};

UCLASS()
class PROJECTMETRUSH_API UPMAttackSlotSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

    // Slot Method
public:
    /** 슬롯 요청 성공 정보 */
    bool TryAcquireSlot(EPMAttackSlotType SlotType, AActor* Requester);

    /** 슬롯 반환 -> 공격 종료/취소/경직/사망 시 호출 */
    void ReleaseSlot(EPMAttackSlotType SlotType, AActor* Requester);

    /** 슬롯 확인 */
    bool IsHolding(EPMAttackSlotType SlotType, AActor* Actor) const;

private:
    /** 사망했거나 요청이 없을 시 슬롯 정리 */
    void CleanUp(EPMAttackSlotType SlotType);

    /** 슬롯 종류별 최대 개수 */
    int32 GetCapacity(EPMAttackSlotType SlotType) const;

    /** 현재 슬롯 사용량 */
    void ShowDebug(EPMAttackSlotType SlotType);

    // Slot Variable
private:
    /** 종류별 현재 슬롯 보유자 목록 */
    TMap<EPMAttackSlotType, TArray<TWeakObjectPtr<AActor>>> Holders;

    /** 종류별 대기열 */
    TMap<EPMAttackSlotType, TArray<FPMSlotWaiter>> Waiters;
};