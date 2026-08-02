#pragma once

#include "CoreMinimal.h"
#include "ProjectMetrush/Character/PMCharacter.h"
#include "PMPlayer.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UPMCombatComponent;
class UInputMappingContext;
class UInputAction;
class UPMStatComponent;
class UDataTable;
struct FInputActionValue;

UCLASS()
class PROJECTMETRUSH_API APMPlayer : public APMCharacter
{
	GENERATED_BODY()

	// Character Interface
public:
	APMPlayer(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// Combat Method
public:
	/** 모든 공격(몬스터, 투사체, 더미)의 피해가 도착하는 곳입니다. 무적/저스트 판단을 여기서 일괄 처리합니다. */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** 현재 체력 비율 (0~1)입니다. 혈투 등 HP 비례 효과 계산용. */
	float GetHPRatio() const { return MaxHP > 0.f ? CurrentHP / MaxHP : 0.f; }

	/** 테스트 치트: 증강 테이블의 해당 행을 즉시 획득합니다. 콘솔에서 AddAugment A004 */
	UFUNCTION(Exec)
	void AddAugment(FName RowName);

private:
	/** 사망 처리 */
	void Die();

	// Input Method
private:
	/** WASD 입력을 카메라 기준 방향으로 변환해 캐릭터를 이동시킵니다. */
	void Input_Move(const FInputActionValue& InputActionValue);

	/** 회피 입력을 전투 컴포넌트로 전달합니다. */
	void Input_Dodge(const FInputActionValue& InputActionValue);

	/** 공격 입력을 전투 컴포넌트로 전달합니다. */
	void Input_Attack(const FInputActionValue& InputActionValue);

	// Component
protected:
	/** 캐릭터와 카메라 사이 거리, 각도를 유지하는 지지대입니다. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	/** 쿼터뷰 시점 카메라입니다. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> CameraComponent;

	/** 전투 상태 머신과 공격, 회피 실행을 담당하는 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPMCombatComponent> CombatComponent;

	/** 보유 증강을 관리하는 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPMStatComponent> StatComponent;

	// Stats Variable
protected:
	/** 최대 체력입니다. 미결정 수치로 임시값 사용, 기획서 확정 필요 */
	UPROPERTY(EditDefaultsOnly, Category = "변수|스탯")
	float MaxHP = 100.f;

	/** 증강 정의 테이블입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|스탯")
	TObjectPtr<UDataTable> AugmentTable;

	/** 현재 체력입니다. */
	UPROPERTY(VisibleInstanceOnly, Category = "변수|스탯")
	float CurrentHP = 0.f;

	// Input Variable
protected:
	/** 게임 시작 시 플레이어에게 적용되는 기본 키 매핑 세트입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|입력")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** 이동 입력 액션입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|입력")
	TObjectPtr<UInputAction> MoveAction;

	/** 회피 입력 액션입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|입력")
	TObjectPtr<UInputAction> DodgeAction;

	/** 기본 공격 입력 액션입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|입력")
	TObjectPtr<UInputAction> AttackAction;

private:
	/** 마지막 유효 이동 입력 방향. 무방향 회피/공격의 방향 기준 (기획서 5.2, 7.1) */
	FVector LastMoveDirection = FVector::ForwardVector;
};