#pragma once

#include "CoreMinimal.h"

namespace PMBlackboardKeys
{
    /** 현재 타겟 플레이어 액터 */
    inline const FName TargetActor = TEXT("TargetActor");

    /** 처음 배치 위치, 추격 종료 후 복귀 위치 */
    inline const FName HomeLocation = TEXT("HomeLocation");

    /** 플레이어를 마지막으로 본 위치 */
    inline const FName LastKnownLocation = TEXT("LastKnownLocation");

    /** 슬롯 대기 위치 */
    inline const FName WaitLocation = TEXT("WaitLocation");

    /** 경직 여부 */
    inline const FName IsStaggered = TEXT("IsStaggered");

    /** 원거리형 사격 위치 */
    inline const FName FirePosition = TEXT("FirePosition");

    /** 원거리형 후퇴 위치 */
    inline const FName RetreatLocation = TEXT("RetreatLocation");

    /** 방패형 위치 */
    inline const FName GuardPosition = TEXT("GuardPosition");

    /** 힐러형 회복 대상 */
    inline const FName HealTarget = TEXT("HealTarget");
}