#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PMStatTypes.generated.h"

/** 프로토타입 증강의 효과 종류입니다. 기획서의 A넘버와 1:1 대응. */
UENUM(BlueprintType)
enum class EPMAugmentEffect : uint8
{
	FinalHitDamage,			// A004 마지막 일격: 3타 피해 증가
	JustDodgeFirstAttack,	// A014 공격적 회피: 저스트 회피 후 첫 공격 강화
	JustDodgeEnemySlow,		// A013 완벽 회피: 저스트 회피 시 적 감속
	JustDodgeCooldownRefund,// A016 회피 충전: 저스트 회피 시 쿨타임 회복
	LowHPDamage				// A035 혈투: 잃은 체력 비례 피해 증가
};

/** 증강 한 종의 정의입니다. DataTable의 행 하나 = 증강 하나. */
USTRUCT(BlueprintType)
struct FPMAugmentData : public FTableRowBase
{
	GENERATED_BODY()

	/** UI에 표시될 증강 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	/** UI에 표시될 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	/** 효과 종류 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EPMAugmentEffect Effect = EPMAugmentEffect::FinalHitDamage;

	/** 단계별 수치. [0]=1단계, [1]=2단계, [2]=3단계. 0.2 = 20% 의미 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<float> StageValues = { 0.2f, 0.35f, 0.5f };
};