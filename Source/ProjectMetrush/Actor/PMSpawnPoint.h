#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Define/PMEnemyType.h"
#include "PMSpawnPoint.generated.h"

class UArrowComponent;

UCLASS()
class PROJECTMETRUSH_API APMSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	APMSpawnPoint();

	// Spawn Method
public:
	/** 이 위치에서 해당 종류를 스폰할 수 있는지 */
	bool CanSpawnType(EPMEnemyType SpawnType) const;

	// Component
protected:
	/** 위치 기준점 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	/** 스폰된 몬스터가 바라볼 방향 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UArrowComponent> DirectionArrow;

	// Spawn Variable
protected:
	/** 이 위치를 쓸 수 있는 몬스터 종류, 비워두면 전부 허용 */
	UPROPERTY(EditAnywhere, Category = "Variable|Spawn")
	TArray<EPMEnemyType> AllowedTypes;
};