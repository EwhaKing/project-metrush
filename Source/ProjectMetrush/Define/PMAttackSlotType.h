#pragma once

#include "CoreMinimal.h"
#include "PMAttackSlotType.generated.h"

/** 공격 슬롯 : 근접형 3 / 원거리형 2 / 방패형 1 / 힐러형 1 */
UENUM(BlueprintType)
enum class EPMAttackSlotType : uint8
{
    Melee,
    Ranged,
    ShieldBash,
    Heal
};