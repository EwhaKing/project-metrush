#include "PMSpawnPoint.h"

#include "Components/ArrowComponent.h"

APMSpawnPoint::APMSpawnPoint()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    DirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("DirectionArrow"));
    DirectionArrow->SetupAttachment(SceneRoot);
}

bool APMSpawnPoint::CanSpawnType(EPMEnemyType SpawnType) const
{
    // 비워두면 모든 몬스터 가능
    if (AllowedTypes.Num() == 0)
    {
        return true;
    }

    return AllowedTypes.Contains(SpawnType);
}