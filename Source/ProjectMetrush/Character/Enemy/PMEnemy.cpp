#include "PMEnemy.h"

#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Character/Enemy/PMEnemyAIController.h"
#include "TimerManager.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Define/PMBlackboardKeys.h"
#include "Subsystem/PMAttackSlotSubsystem.h"

APMEnemy::APMEnemy()
{
    // 몬스터 액터 tick은 사용하지 않음, 대신 BT 사용
    PrimaryActorTick.bCanEverTick = false;

    // 임시로 큐브메시 사용
    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    BodyMesh->SetupAttachment(GetCapsuleComponent());
    BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 레벨에 배치, 스폰 시 자동으로 AI 컨트롤러를 사용
    AIControllerClass = APMEnemyAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void APMEnemy::BeginPlay()
{
    Super::BeginPlay();

    CurrentHP = MaxHP;
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void APMEnemy::SetBodyColor(const FLinearColor& NewColor)
{
    // 처음 호출 시 dynamic material instance 생성
    if (BodyMID == nullptr && BodyMesh != nullptr)
    {
        BodyMID = BodyMesh->CreateAndSetMaterialInstanceDynamic(0);
    }

    if (BodyMID == nullptr)
    {
        return;
    }
    
    BodyMID->SetVectorParameterValue(TEXT("Color"), NewColor);
}

float APMEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    const float BaseDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // 사망 여부 확인
    if (bIsDead || BaseDamage <= 0.f)
    {
        return 0.f;
    }

    float ActualDamage = BaseDamage;

    // 방패형 : 피격 방향에 따라 피해 배율 적용
    if (bHasDirectionalGuard)
    {
        ActualDamage *= GetDirectionalDamageMultiplier(DamageCauser, EventInstigator);
    }

    // 정면 완전 방어 시 피해 없음
    if (ActualDamage <= 0.f)
    {
        UE_LOG(LogTemp, Log, TEXT("%s Front Guarded"), *GetName());
        return 0.f;
    }

    CurrentHP -= ActualDamage;

    // 피격 판정 (힐러 회복 취소)
    ++DamagedCount;

    UE_LOG(LogTemp, Log, TEXT("%s HP: %.0f"), *GetName(), CurrentHP);

    // 추후 조정필요 : 피격 경직/넉백/그로기 미구현. 플레이어 공격이 경직값을 넘겨주면 EnterStagger 호출로 연결
    if (CurrentHP <= 0.f)
    {
        Die();
    }

    return ActualDamage;
}

void APMEnemy::EnterStagger(float StaggerDuration)
{
    // 사망 여부 확인
    if (bIsDead || StaggerDuration <= 0.f)
    {
        return;
    }

    AAIController* AIController = Cast<AAIController>(GetController());
    UBlackboardComponent* BlackboardComp = AIController ? AIController->GetBlackboardComponent() : nullptr;
    if (BlackboardComp == nullptr)
    {
        return;
    }

    // BT 경직 분기가 다른 행동 중단
    BlackboardComp->SetValueAsBool(PMBlackboardKeys::IsStaggered, true);

    // 경직 중 또 경직되면 타이머 갱신
    GetWorldTimerManager().SetTimer(StaggerTimerHandle, this, &APMEnemy::EndStagger, StaggerDuration, false);
}

float APMEnemy::Heal(float HealAmount)
{
    // 죽은 몬스터는 되살리지 않음
    if (bIsDead || HealAmount <= 0.f)
    {
        return 0.f;
    }

    const float BeforeHP = CurrentHP;
    CurrentHP = FMath::Min(CurrentHP + HealAmount, MaxHP);

    const float HealedAmount = CurrentHP - BeforeHP;
    if (HealedAmount > 0.f)
    {
        UE_LOG(LogTemp, Log, TEXT("%s Healed +%.0f -> HP: %.0f"), *GetName(), HealedAmount, CurrentHP);
    }

    // 추후 조정필요 : 회복 이펙트, 회복 사운드
    return HealedAmount;
}

void APMEnemy::SetSpawnAttackBlock(float BlockTime)
{
    if (BlockTime <= 0.f)
    {
        return;
    }

    // 등장 직후 바로 때리지 않게 유예 시간 주기
    AttackBlockedUntilTime = GetWorld()->GetTimeSeconds() + BlockTime;
}

bool APMEnemy::CanAttack() const
{
    return GetWorld()->GetTimeSeconds() >= AttackBlockedUntilTime;
}

void APMEnemy::EndStagger()
{
    AAIController* AIController = Cast<AAIController>(GetController());
    UBlackboardComponent* BlackboardComp = AIController ? AIController->GetBlackboardComponent() : nullptr;
    if (BlackboardComp != nullptr)
    {
        BlackboardComp->SetValueAsBool(PMBlackboardKeys::IsStaggered, false);
    }
}

void APMEnemy::OnParried()
{
    // 패링은 공격 슬롯을 보유한 동안에만 성립
    UPMAttackSlotSubsystem* SlotSubsystem = GetWorld()->GetSubsystem<UPMAttackSlotSubsystem>();
    if (SlotSubsystem == nullptr)
    {
        return;
    }

    const bool bHolding =
        SlotSubsystem->IsHolding(EPMAttackSlotType::Melee, this) ||
        SlotSubsystem->IsHolding(EPMAttackSlotType::ShieldBash, this);
    if (bHolding == false)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("%s Parried"), *GetName());
    EnterStagger(ParryStaggerTime);
}

void APMEnemy::Die()
{
    if (bIsDead)
    {
        return;
    }
    bIsDead = true;
    CurrentHP = 0.f;

    // 진행 중이던 경직 타이머 정리
    GetWorldTimerManager().ClearTimer(StaggerTimerHandle);

    // AI 정지. BT가 멈추면 실행 중이던 공격 태스크가 Abort되어 슬롯 반환
    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController != nullptr && AIController->GetBrainComponent() != nullptr)
    {
        AIController->GetBrainComponent()->StopLogic(TEXT("Dead"));
    }

    UE_LOG(LogTemp, Warning, TEXT("%s Died"), *GetName());

    // 추후 조정필요 : 사망 연출
    Destroy();
}

float APMEnemy::GetDirectionalDamageMultiplier(AActor* DamageCauser, AController* EventInstigator) const
{
    // 추후 조정필요 : 플레이어 공격이 ApplyDamage 호출 시 DamageCauser(또는 Instigator)에 플레이어를 넘겨줘야함
    AActor* Attacker = DamageCauser;
    if (Attacker == nullptr && EventInstigator != nullptr)
    {
        Attacker = EventInstigator->GetPawn();
    }
    if (Attacker == nullptr)
    {
        // 공격자 위치를 모르면 방향 판정 없이 정상 피해
        UE_LOG(LogTemp, Warning, TEXT("[Shield] DamageCauser 없음 - 방향 방어 스킵 (플레이어 ApplyDamage 연동 필요)"));
        return 1.f;
    }

    // 방향 판별
    const FVector ToAttacker = (Attacker->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
    const FVector Forward = GetActorForwardVector().GetSafeNormal2D();
    const float Dot = FVector::DotProduct(Forward, ToAttacker);

    const float FrontCos = FMath::Cos(FMath::DegreesToRadians(FrontGuardAngle * 0.5f));
    if (Dot >= FrontCos)
    {
        return FrontDamageMultiplier;
    }

    // 추후 조정필요 : 후방 각도 120도로 가정
    if (Dot <= -FrontCos)
    {
        return BackDamageMultiplier;
    }

    return 1.f;
}