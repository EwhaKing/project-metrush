#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectMetrush/Define/PMStatTypes.h"
#include "PMStatComponent.generated.h"

/** 보유 중인 증강 하나의 상태입니다. */
USTRUCT()
struct FPMOwnedAugment
{
	GENERATED_BODY()

	/** DataTable 행 이름 (증강 식별자) */
	UPROPERTY()
	FName RowName;

	/** 효과 종류 */
	UPROPERTY()
	EPMAugmentEffect Effect = EPMAugmentEffect::FinalHitDamage;

	/** 현재 단계 (1~3) */
	UPROPERTY()
	int32 Stage = 1;

	/** 단계별 수치 사본 */
	UPROPERTY()
	TArray<float> StageValues;
};

/**
 * 보유 증강과 단계를 관리하는 컴포넌트입니다.
 * 획득/강화는 AcquireAugment, 전투 코드의 수치 질의는 GetEffectValue를 사용합니다.
 */
UCLASS()
class PROJECTMETRUSH_API UPMStatComponent : public UActorComponent
{
	GENERATED_BODY()

	// Augment Method
public:
	/** 증강을 획득합니다. 이미 보유 중이면 1단계 강화합니다 (최대 3단계). 기획서 13장 */
	void AcquireAugment(FName RowName, const FPMAugmentData& Data);

	/** 해당 효과를 보유 중인지 반환합니다. */
	bool HasEffect(EPMAugmentEffect Effect) const;

	/** 해당 효과의 현재 단계 수치를 반환합니다. 미보유 시 0. */
	float GetEffectValue(EPMAugmentEffect Effect) const;

	/** 보유 증강 목록입니다. (추후 UI와 시너지 검사용) */
	const TArray<FPMOwnedAugment>& GetOwnedAugments() const { return OwnedAugments; }

	// Augment Variable
private:
	/** 보유 증강 목록 */
	UPROPERTY()
	TArray<FPMOwnedAugment> OwnedAugments;
};