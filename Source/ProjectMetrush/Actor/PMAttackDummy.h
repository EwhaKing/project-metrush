#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PMAttackDummy.generated.h"

class UStaticMeshComponent;

/** 더미의 공격 사이클 단계입니다. */
UENUM()
enum class EPMDummyPhase : uint8
{
	Idle,		// 대기
	Telegraph,	// 공격 예고 (몬스터 텔레그래프의 임시 대역)
	Recover		// 공격 후 휴식
};

/**
 * 일정 주기로 전방에 공격 판정을 발생시키는 테스트용 더미입니다.
 * 몬스터 구현 전까지 저스트 회피, 패링, 피격 개발에 사용합니다.
 */
UCLASS()
class PROJECTMETRUSH_API APMAttackDummy : public AActor
{
	GENERATED_BODY()

	// Actor Interface
public:
	APMAttackDummy();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Attack Method
private:
	/** 전방 판정 범위 안의 플레이어를 찾아 명중/회피 결과를 처리합니다. */
	void PerformAttack();

	/** 현재 단계에 맞는 텔레그래프(디버그 박스)를 그립니다. */
	void DrawPhaseDebug() const;

	// Component
protected:
	/** 더미의 몸체 메시입니다. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	// Attack Variable
protected:
	/** 공격과 공격 사이의 대기 시간(초)입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|더미")
	float IdleDuration = 2.0f;

	/** 공격 예고(텔레그래프) 시간(초)입니다. 이 시간이 끝나는 순간 판정이 발생합니다. */
	UPROPERTY(EditAnywhere, Category = "변수|더미")
	float TelegraphDuration = 1.0f;

	/** 공격 후 휴식 시간(초)입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|더미")
	float RecoverDuration = 1.0f;

	/** 공격 판정의 전방 거리(cm)입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|더미")
	float AttackRange = 250.f;

	/** 공격 판정의 좌우 폭(cm)입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|더미")
	float AttackWidth = 200.f;

	/** 공격 피해량입니다. (HP 구현 전까지는 표시용) */
	UPROPERTY(EditAnywhere, Category = "변수|더미")
	float Damage = 10.f;

private:
	EPMDummyPhase Phase = EPMDummyPhase::Idle;
	float PhaseElapsedTime = 0.f;
};