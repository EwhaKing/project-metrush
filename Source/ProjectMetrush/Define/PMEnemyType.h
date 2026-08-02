#pragma once

#include "CoreMinimal.h"
#include "PMEnemyType.generated.h"

/** 몬스터 종류 */
UENUM(BlueprintType)
enum class EPMEnemyType : uint8
{
    Melee,
    Ranged,
    Shield,
    Healer
};