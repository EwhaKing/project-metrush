#pragma once

#include "CoreMinimal.h"
#include "ProjectMetrush/Character/PMCharacter.h"
#include "PMPlayer.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UPMCombatComponent;
class UInputMappingContext;
class UInputAction;
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