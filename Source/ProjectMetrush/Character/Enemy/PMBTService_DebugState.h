#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "PMBTService_DebugState.generated.h"

UCLASS()
class PROJECTMETRUSH_API UPMBTService_DebugState : public UBTService
{
	GENERATED_BODY()

public:
	UPMBTService_DebugState();

protected:
	/** 인터벌마다 몬스터 상태와 범위를 화면에 표시 */
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// Debug Variable
protected:

	/** 인지 범위 표시 */
	UPROPERTY(EditAnywhere, Category = "Variable|Debug")
	float DetectRadius = 1000.f;

	/** 공격 범위 표시 */
	UPROPERTY(EditAnywhere, Category = "Variable|Debug")
	float AttackRange = 180.f;
};