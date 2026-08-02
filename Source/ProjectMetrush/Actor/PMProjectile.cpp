#include "PMProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Character/Enemy/PMEnemy.h"
#include "Kismet/GameplayStatics.h"

APMProjectile::APMProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    // 충돌 구체: 벽은 막히고, 폰과는 겹침
    SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
    SphereComponent->InitSphereRadius(15.f);      // 추후 조정필요 : 투사체 충돌 반경, 회피 난이도에 영향
    SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
    SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    SphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    RootComponent = SphereComponent;

    // 외형
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(SphereComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 직선 비행 설정
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = 1200.f;      // 추후 조정필요 : 투사체 속도
    ProjectileMovement->MaxSpeed = 1200.f;
    ProjectileMovement->ProjectileGravityScale = 0.f;
    ProjectileMovement->bRotationFollowsVelocity = true;

    // 아무것도 못 맞히면 자동 소멸 -> 추후 조정필요 : 유지 시간
    InitialLifeSpan = 3.f;
}

void APMProjectile::BeginPlay()
{
    Super::BeginPlay();

    SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &APMProjectile::OnSphereBeginOverlap);
    SphereComponent->OnComponentHit.AddDynamic(this, &APMProjectile::OnSphereHit);
}

void APMProjectile::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 몬스터는 통과
    if (OtherActor == nullptr || OtherActor == GetInstigator() || Cast<APMEnemy>(OtherActor) != nullptr)
    {
        return;
    }

    if (Cast<APawn>(OtherActor) != nullptr)
    {
        // 피해 전달. 무적/저스트 판단은 플레이어 TakeDamage가 일괄 처리
        UGameplayStatics::ApplyDamage(OtherActor, Damage,
            GetInstigatorController(), this, nullptr);
        Destroy();
    }
}

void APMProjectile::OnSphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // 벽/엄폐물에 막힘
    Destroy();
}