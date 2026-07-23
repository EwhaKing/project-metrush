#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PMProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class PROJECTMETRUSH_API APMProjectile : public AActor
{
    GENERATED_BODY()

public:
    APMProjectile();

    virtual void BeginPlay() override;

    // Collision Method
private:
    /** 폰과 겹치면 -> 몬스터는 통과, 플레이어는 명중 후 소멸 */
    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    /** 막히는 지형에 부딪히면 소멸 */
    UFUNCTION()
    void OnSphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    // Component
protected:
    /** 충돌 판정용 구체 */
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USphereComponent> SphereComponent;

    /** 투사체 외형 */
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    /** 직선 비행 이동 컴포넌트 */
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    // Combat Variable
protected:
    /** 플레이어 명중 시 피해량 */
    UPROPERTY(EditDefaultsOnly, Category = "Variable|Combat")
    float Damage = 10.f;

};