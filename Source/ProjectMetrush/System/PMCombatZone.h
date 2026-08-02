#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Define/PMWaveTypes.h"
#include "PMCombatZone.generated.h"

class UBoxComponent;
class APMEnemy;
class APMSpawnPoint;

UCLASS()
class PROJECTMETRUSH_API APMCombatZone : public AActor
{
	GENERATED_BODY()

public:
	APMCombatZone();

	virtual void BeginPlay() override;

	// Combat Method
protected:
	/** 플레이어가 구역에 진입하면 전투 시작 */
	UFUNCTION()
	void OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	/** 전투 시작 */
	void StartCombat();

	/** 일정 간격으로 생존 확인, 스폰, 웨이브 전환 판정 */
	void UpdateCombat();

	/** 다음 웨이브를 스폰 대기열에 올림 */
	void StartNextWave();

	/** 대기열에서 한 마리씩 스폰 */
	void ProcessPendingSpawns();

	/** 죽었거나 사라진 몬스터를 목록에서 정리 */
	void CleanUpEnemyList();

	/** 웨이브 전환 조건 확인 */
	bool ShouldStartNextWave() const;

	/** 스폰할 위치 선택 */
	APMSpawnPoint* PickSpawnPoint(TSubclassOf<APMEnemy> EnemyClass) const;

	/** 살아있는 힐러 있는지 */
	bool HasAliveHealer() const;

	/** 전투 종료 */
	void EndCombat();

	// Component
protected:
	/** 플레이어 진입 감지용 전투 구역 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> ZoneBox;

	// Wave Variable
protected:
	/** 웨이브 구성 */
	UPROPERTY(EditAnywhere, Category = "Variable|Wave")
	TArray<FPMWaveInfo> Waves;

	/** 이 구역에서 사용할 스폰 포인트 */
	UPROPERTY(EditAnywhere, Category = "Variable|Wave")
	TArray<TObjectPtr<APMSpawnPoint>> SpawnPoints;

	/** 동시 활성 몬스터 최대 수 */
	UPROPERTY(EditAnywhere, Category = "Variable|Wave")
	int32 MaxActiveEnemies = 18;

	/** 다음 웨이브 전환 : 현재 웨이브 남은 수 기준 */
	UPROPERTY(EditAnywhere, Category = "Variable|Wave")
	int32 NextWaveRemainCount = 2;

	/** 다음 웨이브 전환 : 처치 비율 기준 */
	UPROPERTY(EditAnywhere, Category = "Variable|Wave")
	float NextWaveKillRatio = 0.75f;

	/** 일반 웨이브 전환 대기 시간 */
	UPROPERTY(EditAnywhere, Category = "Variable|Wave")
	float WaveTransitionTime = 1.5f;

	/** 마지막 웨이브 예고 시간 */
	UPROPERTY(EditAnywhere, Category = "Variable|Wave")
	float FinalWaveNoticeTime = 2.0f;

	// Spawn Variable
protected:
	/** 플레이어와의 최소 스폰 거리 */
	UPROPERTY(EditAnywhere, Category = "Variable|Spawn")
	float MinSpawnDistanceFromPlayer = 500.f;

	/** 스폰 직후 공격 금지 시간 */
	UPROPERTY(EditAnywhere, Category = "Variable|Spawn")
	float SpawnAttackBlockTime = 0.5f;

	/** 전투 갱신 간격 */
	UPROPERTY(EditAnywhere, Category = "Variable|Spawn")
	float UpdateInterval = 0.25f;

	// Runtime State
private:
	/** 전투 진행 중 여부 */
	bool bCombatStarted = false;

	/** 웨이브 전환 대기 중인지 */
	bool bWaitingNextWave = false;

	/** 현재 웨이브 번호 */
	int32 CurrentWaveIndex = -1;

	/** 현재 웨이브에서 스폰하기로 한 총 수 */
	int32 CurrentWaveTotalCount = 0;

	/** 현재 웨이브 시작 시각 */
	float CurrentWaveStartTime = 0.f;

	/** 아직 스폰하지 못한 몬스터 대기열 */
	TArray<TSubclassOf<APMEnemy>> PendingSpawns;

	/** 현재 살아있는 전체 몬스터 -> 동시 활성 수 제한용 */
	TArray<TWeakObjectPtr<APMEnemy>> ActiveEnemies;

	/** 현재 웨이브에서 나온 몬스터 -> 전환 조건 판정용 */
	TArray<TWeakObjectPtr<APMEnemy>> CurrentWaveEnemies;

	/** 전투 갱신 타이머 */
	FTimerHandle UpdateTimerHandle;

	/** 웨이브 전환 타이머 */
	FTimerHandle WaveTimerHandle;
};