#pragma once

#include "CoreMinimal.h"
#include "ProjectMetrush/Character/PMCharacter.h"
#include "Define/PMEnemyType.h"
#include "PMEnemy.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UBehaviorTree;

UCLASS()
class PROJECTMETRUSH_API APMEnemy : public APMCharacter
{
	GENERATED_BODY()

public:
	APMEnemy();

	virtual void BeginPlay() override;

	// AI method
public:
	/** 몬스터가 실행할 behavior tree */
	UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

	// Type method
public:
	/** 몬스터 종류 */
	EPMEnemyType GetEnemyType() const { return EnemyType; }

	// Telegraph method
public:
	/** 몬스터 body 색 바꾸기 텔레그래프 */
	UFUNCTION(BlueprintCallable, Category = "PM|Telegraph")
	void SetBodyColor(const FLinearColor& NewColor);

	// Combat method
public:
	/** 데미지 처리 */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** 경직 시작 */
	UFUNCTION(BlueprintCallable, Category = "PM|Combat")
	void EnterStagger(float StaggerDuration);

	/** 패링 */
	UFUNCTION(BlueprintCallable, Category = "PM|Combat")
	void OnParried();

	/** 사망 여부 */
	bool IsDead() const { return bIsDead; }

	/** 스폰 직후 일정 시간 공격 금지 */
	void SetSpawnAttackBlock(float BlockTime);

	/** 지금 공격할 수 있는지 -> 공격 Task가 시작 전에 확인 */
	bool CanAttack() const;

	/** 최대 체력 */
	float GetMaxHP() const { return MaxHP; }

	/** 현재 체력 비율 -> 힐러형이 가장 다친 아군을 고를 때 사용 */
	float GetHPRatio() const { return MaxHP > 0.f ? CurrentHP / MaxHP : 0.f; }

	/** 체력 회복 -> 실제로 회복된 양 */
	float Heal(float HealAmount);

	/** 지금까지 피격당한 횟수 -> 시전 도중 맞았는지 판정용 */
	int32 GetDamagedCount() const { return DamagedCount; }

private:
	/**사망 처리 */
	void Die();

	/** 경직 종료 */
	void EndStagger();

	/** 피격 방향 별 피해 배율 */
	float GetDirectionalDamageMultiplier(AActor* DamageCauser, AController* EventInstigator) const;

	// Component
protected:
	/** 몬스터 외형용 메시 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	// AI Variable
protected:
	/** 몬스터가 실행할 behavior tree */
	UPROPERTY(EditDefaultsOnly, Category = "Variable|AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	// Stats Variable
protected:
	/** 몬스터 최대체력 -> 추후 조정필요*/
	UPROPERTY(EditDefaultsOnly, Category = "Variable|Stats")
	float MaxHP = 100.f;

	/** 몬스터 현재체력 */
	UPROPERTY(VisibleInstanceOnly, Category = "Variable|Stats")
	float CurrentHP = 0.f;

	/** 몬스터 이동 속도 (cm/s) -> 추후 조정필요 */
	UPROPERTY(EditDefaultsOnly, Category = "Variable|Stats")
	float MoveSpeed = 400.f;

	/** 패링 경직 시간 -> 추후 조정필요 : 플레이어 패링 성공 판정과 길이를 맞춰야 함 */
	UPROPERTY(EditDefaultsOnly, Category = "Variable|Stats")
	float ParryStaggerTime = 0.5f;

	// Type Variable
protected:
	/** 몬스터 종류 */
	UPROPERTY(EditDefaultsOnly, Category = "Variable|Type")
	EPMEnemyType EnemyType = EPMEnemyType::Melee;

	// Guard Variable
protected:
	/** 방향 방어 여부 */
	UPROPERTY(EditDefaultsOnly, Category = "Variable|Guard")
	bool bHasDirectionalGuard = false;

	/** 전방 방어 각도 -> 추후 조정필요 : 정면 방어 범위가 얼마나 넓어야 하는지 미정 */
	UPROPERTY(EditDefaultsOnly, Category = "Variable|Guard")
	float FrontGuardAngle = 120.f;

	/** 전방 피격 피해 비율 -> 추후 조정필요 : 현재 완전 무효(0배), 일부만 감소시킬지 미정 */
	UPROPERTY(EditDefaultsOnly, Category = "Variable|Guard")
	float FrontDamageMultiplier = 0.f;

	/** 후방 피격 피해 비율 -> 추후 조정필요 : 후방 보너스 배율 미정 */
	UPROPERTY(EditDefaultsOnly, Category = "Variable|Guard")
	float BackDamageMultiplier = 1.5f;

	// Combat Stats
private:
	/** 사망 여부 */
	bool bIsDead = false;

	/** 피격당한 횟수 */
	int32 DamagedCount = 0;

	/** 이 시각까지 공격 금지 */
	float AttackBlockedUntilTime = 0.f;

	/** 경직 종료 timer handle */
	FTimerHandle StaggerTimerHandle;

	// Telegraph Variable
protected:
	/** 다이나믹 머터리얼 인스턴스 */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BodyMID;
};
