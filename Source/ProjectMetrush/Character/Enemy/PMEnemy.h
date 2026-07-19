#pragma once

#include "CoreMinimal.h"
#include "ProjectMetrush/Character/PMCharacter.h"
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

	/** 패링 -> 추후 조정필요 : 플레이어 파트와 연결 */
	UFUNCTION(BlueprintCallable, Category = "PM|Combat")
	void OnParried();

	/** 사망 여부 */
	bool IsDead() const { return bIsDead; }

private:
	/**사망 처리 */
	void Die();

	/** 경직 종료 */
	void EndStagger();

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

	/** 패링 경직 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Variable|Stats")
	float ParryStaggerTime = 0.5f;

	// Combat Stats
private:
	/** 사망 여부 */
	bool bIsDead = false;

	/** 경직 종료 timer handle */
	FTimerHandle StaggerTimerHandle;

	// Telegraph Variable
protected:
	/** 다이나믹 머터리얼 인스턴스 */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BodyMID;
};
