#pragma once

#include "CoreMinimal.h"
#include "PMWaveTypes.generated.h"

class APMEnemy;

/** 웨이브에 등장하는 몬스터 한 종류의 구성 */
USTRUCT(BlueprintType)
struct FPMWaveEntry
{
    GENERATED_BODY()

    /** 등장할 몬스터 블루프린트 */
    UPROPERTY(EditAnywhere, Category = "Variable|Wave")
    TSubclassOf<APMEnemy> EnemyClass;

    /** 등장 수 */
    UPROPERTY(EditAnywhere, Category = "Variable|Wave")
    int32 Count = 1;
};

/** 웨이브 한 번의 구성 */
USTRUCT(BlueprintType)
struct FPMWaveInfo
{
    GENERATED_BODY()

    /** 이번 웨이브에 등장할 몬스터 목록 */
    UPROPERTY(EditAnywhere, Category = "Variable|Wave")
    TArray<FPMWaveEntry> Entries;

    /** 제한시간이 지나면 다음 웨이브로 전환, 0이면 시간 조건 사용 안 함 -> 추후 조정필요 : 구체적 초 지정 */
    UPROPERTY(EditAnywhere, Category = "Variable|Wave")
    float TimeLimit = 0.f;
};