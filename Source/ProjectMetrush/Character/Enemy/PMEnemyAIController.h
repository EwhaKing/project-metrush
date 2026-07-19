#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PMEnemyAIController.generated.h"

UCLASS()
class PROJECTMETRUSH_API APMEnemyAIController : public AAIController
{
    GENERATED_BODY()

protected:
    /** pawn에 빙의 시 호출, behavior tree 실행 */
    virtual void OnPossess(APawn* InPawn) override;
};