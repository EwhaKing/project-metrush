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

float APMEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // 사망 여부 확인
    if (bIsDead || ActualDamage <= 0.f)
    {
        return 0.f;
    }

    CurrentHP -= ActualDamage;
    UE_LOG(LogTemp, Log, TEXT("%s HP: %.0f"), *GetName(), CurrentHP);
    // 추후 조정필요 : 피격 연출

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
    // 패링 전 공격 중인지 확인
    // 추후 조정필요 : 방패형 등 다른 슬롯 종류가 생기면 검사 대상 확장
    UPMAttackSlotSubsystem* SlotSubsystem = GetWorld()->GetSubsystem<UPMAttackSlotSubsystem>();
    if (SlotSubsystem == nullptr || SlotSubsystem->IsHolding(EPMAttackSlotType::Melee, this) == false)
    {
        return;
    }

    // (이하 기존 내용 그대로)
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